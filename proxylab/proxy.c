#include "csapp.h"
#include "cache.h"

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* 从请求 URL 中解析出的目标服务器和资源路径。 */
typedef struct {
    char host[MAXLINE]; /* 目标主机名或 IP，如 www.cmu.edu；不含 http:// 和端口。 */
    char port[MAXLINE]; /* 目标服务器端口，以字符串保存，如 "8080"；URL 未指定时为 "80"。 */
    char path[MAXLINE]; /* 请求的资源路径，保留查询参数，如 /search?q=csapp；无路径时为 /。 */
} target_t;

/* 检查命令行参数并输出错误提示；成功返回 0，失败返回 -1。 */
int validate_args(int argc, char **argv);

/* 线程例程：arg 指向堆上分配的 int，接管参数内存及客户端连接。 */
void *worker(void *arg);

/* 处理一次请求，关闭目标服务器连接；客户端连接由调用方关闭。 */
void handle_client(int clientfd);

/* 检查请求行、解析 URL 并构造转发请求；成功返回 0，失败返回 -1。 */
int prepare_request(rio_t *client_rio, const char *request_line,
                    target_t *target, char *request, size_t capacity);

/* 解析 URL，默认端口为 80、路径为 /；成功返回 0，失败返回 -1。 */
int parse_uri(const char *uri, target_t *target);

/* 读完请求头并构造 HTTP/1.0 请求；成功返回 0，失败返回 -1。 */
int build_request(rio_t *client_rio, const target_t *target,
                  char *request, size_t capacity);

/* 连接并发送请求；失败时发送错误响应并返回 -1，成功返回由调用方关闭的服务器描述符。 */
int send_request(int clientfd, target_t *target, char *request);

/* 转发服务器响应，完整接收且未超限时写入缓存；成功返回 0，失败返回 -1。 */
int forward_response(int serverfd, int clientfd, const char *key);

/* 向客户端发送 HTTP 错误响应。 */
void send_error(int clientfd, int status,
                const char *reason, const char *message);

int main(int argc, char **argv)
{
    /* 1. 检查命令行参数：用法为 ./proxy <监听端口>，参数无效时提示并退出。 */
    if (validate_args(argc, argv) < 0) {
        return EXIT_FAILURE;
    }

    /* 2. 忽略 SIGPIPE，避免客户端提前断开后，写操作导致整个代理退出。 */
    Signal(SIGPIPE, SIG_IGN);

    /* 在工作线程启动前初始化共享缓存和信号量。 */
    if (cache_init() < 0) {
        perror("cache_init error");
        return EXIT_FAILURE;
    }

    /* 3. 在 argv[1] 指定的端口创建监听套接字，失败时提示并退出。 */
    int listenfd = Open_listenfd(argv[1]);

    /* 4. 循环接受连接，每个连接交给独立线程处理。 */
    while (1) {
        /* a. 重置客户端地址长度，等待并接受连接，获得客户端连接描述符。 */
        struct sockaddr_storage clientaddr;
        socklen_t clientlen = sizeof(clientaddr);
        int clientfd = accept(listenfd, (SA *)&clientaddr, &clientlen);

        /* b. 若接受连接失败，处理错误并继续等待下一次连接。 */
        if(clientfd < 0){
            perror("accept error");
            continue;
        }

        /* c. 为线程单独保存描述符，避免下一次循环覆盖线程参数。 */
        int *clientfd_ptr = malloc(sizeof(*clientfd_ptr));
        if (clientfd_ptr == NULL) {
            perror("malloc error");
            close(clientfd);
            continue;
        }
        *clientfd_ptr = clientfd;

        /* d. 创建线程；成功后由 worker 释放参数并关闭客户端连接。 */
        pthread_t tid;
        int err = pthread_create(&tid, NULL, worker, clientfd_ptr);
        if (err != 0) {
            fprintf(stderr, "pthread_create error: %s\n", strerror(err));
            free(clientfd_ptr);
            close(clientfd);
        }
    }

    return 0;
}

int validate_args(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return -1;
    }

    char *end;
    errno = 0;
    long port = strtol(argv[1], &end, 10);
    if (errno == ERANGE || end == argv[1] || *end != '\0' ||
        port <= 1024 || port > 65535) {
        fprintf(stderr, "Invalid port: %s (expected an integer from 1025 to 65535)\n",
                argv[1]);
        return -1;
    }

    return 0;
}

void *worker(void *arg)
{
    /* 1. 保存连接描述符，释放创建线程时分配的参数内存。 */
    int clientfd = *(int *)arg;
    free(arg);

    /* 2. 分离当前线程，结束后自动回收线程资源，无需 pthread_join。 */
    pthread_detach(pthread_self());

    /* 3. 完成请求和响应转发，再关闭由本线程负责的客户端连接。 */
    handle_client(clientfd);
    close(clientfd);
    return NULL;
}

void handle_client(int clientfd)
{
    /* 1. 初始化 RIO，读取客户端请求行。 */
    rio_t client_rio;
    char request_line[MAXLINE];

    rio_readinitb(&client_rio, clientfd);
    if (rio_readlineb(&client_rio, request_line, sizeof(request_line)) <= 0) {
        return;
    }

    /* 2. 解析并检查请求，构造发给目标服务器的 HTTP/1.0 请求。 */
    target_t target;
    char request[MAXLINE * 4];
    if (prepare_request(&client_rio, request_line, &target,
                        request, sizeof(request)) < 0) {
        return;
    }

    /* 3. 使用主机、端口和完整路径作为缓存标识；命中时直接发送响应副本。 */
    char key[MAXLINE * 3 + 32];
    int key_len = snprintf(key, sizeof(key), "http://[%s]:%s%s",
                           target.host, target.port, target.path);
    if (key_len < 0 || (size_t)key_len >= sizeof(key)) {
        send_error(clientfd, 400, "Bad Request", "URL is too long.");
        return;
    }
    unsigned char *data = NULL;
    size_t size = 0;
    if (cache_get(key, &data, &size) == 1) {
        rio_writen(clientfd, data, size);
        free(data);
        return;
    }

    /* 4. 未命中或缓存查询失败时，仍正常连接服务器并转发请求。 */
    int serverfd = send_request(clientfd, &target, request);
    if (serverfd < 0) {
        return;
    }

    /* 5. 转发响应，同时收集可缓存的完整对象。 */
    forward_response(serverfd, clientfd, key);

    /* 6. 关闭目标服务器连接；客户端连接由调用方关闭。 */
    close(serverfd);
}

int send_request(int clientfd, target_t *target, char *request)
{
    int serverfd = open_clientfd(target->host, target->port);
    if (serverfd < 0) {
        send_error(clientfd, 502, "Bad Gateway", "Cannot connect to server.");
        return -1;
    }

    size_t request_len = strlen(request);
    if (rio_writen(serverfd, request, request_len) != (ssize_t)request_len) {
        close(serverfd);
        send_error(clientfd, 502, "Bad Gateway", "Cannot send request to server.");
        return -1;
    }

    return serverfd;
}

int prepare_request(rio_t *client_rio, const char *request_line,
                    target_t *target, char *request, size_t capacity)
{
    int clientfd = client_rio->rio_fd;
    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char extra;

    /* a. 请求行必须完整，且只包含方法、URL、版本三个字段。 */
    size_t line_len = strlen(request_line);
    if (line_len < 2 || strcmp(request_line + line_len - 2, "\r\n") != 0 ||
        sscanf(request_line, "%8191s %8191s %8191s %c",
               method, uri, version, &extra) != 3) {
        send_error(clientfd, 400, "Bad Request", "Invalid request line.");
        return -1;
    }

    if (strcmp(version, "HTTP/1.0") != 0 && strcmp(version, "HTTP/1.1") != 0) {
        send_error(clientfd, 400, "Bad Request", "Unsupported HTTP version.");
        return -1;
    }

    if (strcmp(method, "GET") != 0) {
        send_error(clientfd, 501, "Not Implemented", "Only GET is supported.");
        return -1;
    }

    /* b. 解析目标地址。 */
    if (parse_uri(uri, target) < 0) {
        send_error(clientfd, 400, "Bad Request", "Invalid URL.");
        return -1;
    }

    /* c. 读取请求头，构造转发请求。 */
    if (build_request(client_rio, target, request, capacity) < 0) {
        send_error(clientfd, 400, "Bad Request", "Cannot build request.");
        return -1;
    }

    return 0;
}

/* 解析可选端口，未指定时使用 80。 */
static int parse_port(const char *port, const char *end,
                      char *result, size_t capacity)
{
    if (port == NULL) {
        strcpy(result, "80");
        return 0;
    }

    size_t len = (size_t)(end - port);
    unsigned int value = 0;
    if (len == 0 || len >= capacity) {
        return -1;
    }
    for (const char *p = port; p < end; ++p) {
        if (*p < '0' || *p > '9') {
            return -1;
        }
        value = value * 10 + (unsigned int)(*p - '0');
        if (value > 65535) {
            return -1;
        }
    }
    if (value == 0) {
        return -1;
    }
    memcpy(result, port, len);
    result[len] = '\0';
    return 0;
}

/* 解析主机和端口，支持方括号包裹的 IPv6 地址。 */
static int parse_authority(const char *host, const char *authority_end,
                           target_t *target)
{
    const char *host_end;
    const char *port = NULL;

    if (memchr(host, '@', (size_t)(authority_end - host)) != NULL) {
        return -1;
    }

    if (*host == '[') {
        ++host;
        host_end = memchr(host, ']', (size_t)(authority_end - host));
        if (host_end == NULL ||
            (host_end + 1 != authority_end && host_end[1] != ':')) {
            return -1;
        }
        if (host_end + 1 != authority_end) {
            port = host_end + 2;
        }
    } else {
        host_end = memchr(host, ':', (size_t)(authority_end - host));
        if (host_end != NULL) {
            port = host_end + 1;
        } else {
            host_end = authority_end;
        }
    }

    size_t host_len = (size_t)(host_end - host);
    if (host_len == 0 || host_len >= sizeof(target->host)) {
        return -1;
    }
    memcpy(target->host, host, host_len);
    target->host[host_len] = '\0';
    return parse_port(port, authority_end, target->port, sizeof(target->port));
}

/* 保留查询参数，去掉片段标识；无路径时补上 /。 */
static int parse_path(const char *start, char *path, size_t capacity)
{
    size_t path_len = strcspn(start, "#");
    size_t prefix_len = (*start == '/') ? 0 : 1;
    if (path_len + prefix_len >= capacity) {
        return -1;
    }
    path[0] = '/';
    memcpy(path + prefix_len, start, path_len);
    path[path_len + prefix_len] = '\0';
    return 0;
}

int parse_uri(const char *uri, target_t *target)
{
    if (uri == NULL || target == NULL || strncasecmp(uri, "http://", 7) != 0) {
        return -1;
    }
    for (const char *p = uri; *p; ++p) {
        if ((unsigned char)*p <= ' ' || (unsigned char)*p == 127) {
            return -1;
        }
    }

    const char *host = uri + 7;
    const char *path = host + strcspn(host, "/?#");
    if (parse_authority(host, path, target) < 0) {
        return -1;
    }
    return parse_path(path, target->path, sizeof(target->path));
}

/* 追加请求内容，始终为字符串结束符留出空间。 */
static int append_request(char *request, size_t capacity, size_t *used,
                          const char *text, size_t length)
{
    if (*used >= capacity || length >= capacity - *used) {
        return -1;
    }
    memcpy(request + *used, text, length);
    *used += length;
    request[*used] = '\0';
    return 0;
}

/* 这些请求头由代理统一设置，不转发客户端提供的值。 */
static int is_fixed_header(const char *name, size_t length)
{
    return (length == 10 && strncasecmp(name, "User-Agent", length) == 0) ||
           (length == 10 && strncasecmp(name, "Connection", length) == 0) ||
           (length == 16 && strncasecmp(name, "Proxy-Connection", length) == 0);
}

int build_request(rio_t *client_rio, const target_t *target,
                  char *request, size_t capacity)
{
    /* 1. 检查输入参数；capacity 包含字符串结束符所需的空间。 */
    if (client_rio == NULL || target == NULL || request == NULL || capacity == 0) {
        return -1;
    }
    /* 2. 使用资源路径构造请求行，并写入代理统一设置的三个请求头。
     *    使用 HTTP/1.0 和 close，让服务器发送完响应后关闭连接。
     */
    int n = snprintf(request, capacity, "GET %s HTTP/1.0\r\n%s"
                     "Connection: close\r\nProxy-Connection: close\r\n",
                     target->path, user_agent_hdr);
    /* snprintf 返回完整输出所需的长度；达到容量说明发生了截断。 */
    if (n < 0 || (size_t)n >= capacity) {
        return -1;
    }
    size_t used = (size_t)n; /* 已写入的字节数，不含末尾的 '\0'。 */
    char line[MAXLINE];
    int has_host = 0;

    /* 3. 逐行读取客户端请求头，直到遇到只包含 CRLF 的空行。 */
    while (1) {
        ssize_t length = rio_readlineb(client_rio, line, sizeof(line));
        /* 拒绝提前断开、读取失败、不完整或过长的行，以及内嵌空字符。 */
        if (length < 2 || line[length - 2] != '\r' ||
            line[length - 1] != '\n' ||
            memchr(line, '\0', (size_t)length) != NULL) {
            return -1;
        }
        if (length == 2) {
            break;
        }
        /* a. 按第一个冒号分隔字段名和值，检查字段名非空且字符合法。 */
        const char *colon = strchr(line, ':');
        if (colon == NULL || colon == line) {
            return -1;
        }
        size_t name_len = (size_t)(colon - line);
        for (const char *p = line; p < colon; ++p) {
            if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
                  (*p >= '0' && *p <= '9') || strchr("!#$%&'*+-.^_`|~", *p))) {
                return -1;
            }
        }
        /* b. 跳过已由代理设置的请求头，避免重复或冲突。 */
        if (is_fixed_header(line, name_len)) {
            continue;
        }
        /* c. 保留客户端提供的 Host，但拒绝重复的 Host 字段。 */
        if (name_len == 4 && strncasecmp(line, "Host", name_len) == 0) {
            if (has_host) {
                return -1;
            }
            has_host = 1;
        }
        /* d. 原样追加其余请求头，空间不足时返回失败。 */
        if (append_request(request, capacity, &used, line, (size_t)length) < 0) {
            return -1;
        }
    }

    /* 4. 客户端未提供 Host 时，根据 URL 补充；IPv6 加方括号，
     *    非默认端口追加 :port，默认端口 80 则省略。
     */
    if (!has_host) {
        int ipv6 = strchr(target->host, ':') != NULL;
        int default_port = strcmp(target->port, "80") == 0;
        n = snprintf(request + used, capacity - used, "Host: %s%s%s%s%s\r\n",
                     ipv6 ? "[" : "", target->host, ipv6 ? "]" : "",
                     default_port ? "" : ":", default_port ? "" : target->port);
        if (n < 0 || (size_t)n >= capacity - used) {
            return -1;
        }
        used += (size_t)n;
    }
    /* 5. 追加空行结束请求头；构造成功返回 0，容量不足返回 -1。 */
    return append_request(request, capacity, &used, "\r\n", 2);
}

int forward_response(int serverfd, int clientfd, const char *key)
{
    /* 1. 准备转发缓冲区和对象缓冲区；内存分配失败时仍正常转发。 */
    rio_t server_rio;
    char buf[MAXBUF];
    unsigned char *object = malloc(MAX_OBJECT_SIZE);
    size_t object_size = 0;
    ssize_t n;

    rio_readinitb(&server_rio, serverfd);

    /* 2. 循环读取服务器响应，每读到一块，就按实际字节数写给客户端。
     *    读取或写入失败时返回 -1；读到 EOF 时结束循环。
     */
    while ((n = rio_readnb(&server_rio, buf, sizeof(buf))) > 0) {
        if (rio_writen(clientfd, buf, (size_t)n) != n) {
            free(object);
            return -1;
        }
        /* 超过对象上限后释放暂存数据，此响应后续只转发、不缓存。 */
        if (object != NULL) {
            if ((size_t)n > MAX_OBJECT_SIZE - object_size) {
                free(object);
                object = NULL;
            } else {
                memcpy(object + object_size, buf, (size_t)n);
                object_size += (size_t)n;
            }
        }
    }
    if (n < 0) {
        free(object);
        return -1;
    }

    /* 3. 正常读到 EOF 才缓存；缓存写入失败不影响已经完成的响应转发。 */
    if (object != NULL && object_size > 0) {
        cache_put(key, object, object_size);
    }
    free(object);
    return 0;
}


void send_error(int clientfd, int status,
                const char *reason, const char *message)
{
    /* 1. 构造纯文本正文，包含状态码、原因和具体错误说明。 */
    char body[MAXBUF];
    int body_len = snprintf(body, sizeof(body), "%d %s\n%s\n",
                            status, reason, message);
    if (body_len < 0 || (size_t)body_len >= sizeof(body)) {
        return;
    }

    /* 2. 正文长度按字节计算；空行分隔响应头和正文。 */
    char headers[MAXLINE];
    int header_len = snprintf(headers, sizeof(headers),
                              "HTTP/1.0 %d %s\r\n"
                              "Content-Type: text/plain; charset=utf-8\r\n"
                              "Content-Length: %d\r\n"
                              "Connection: close\r\n\r\n",
                              status, reason, body_len);
    if (header_len < 0 || (size_t)header_len >= sizeof(headers)) {
        return;
    }

    /* 3. 先发送响应头，再发送正文；写入失败则结束，由调用方关闭连接。 */
    if (rio_writen(clientfd, headers, (size_t)header_len) != header_len) {
        return;
    }
    rio_writen(clientfd, body, (size_t)body_len);
}
