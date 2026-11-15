#include "cache.h"

#include <errno.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

/* 缓存节点只在模块内部使用，proxy.c 通过 cache.h 中的接口访问。 */
typedef struct cache_entry {
    char *key;                       /* 资源标识：主机、端口和路径。 */
    unsigned char *data;             /* 完整 HTTP 响应，支持二进制。 */
    size_t size;                     /* 响应字节数，不使用 strlen 计算。 */
    atomic_ulong last_used;          /* 最近使用序号，允许多个读线程更新。 */
    struct cache_entry *next;
} cache_entry_t;

/* 教材中的读者优先方案：允许并发读取，写入和淘汰独占访问。 */
typedef struct {
    cache_entry_t *head;
    size_t total_size;               /* 只统计响应数据，不包含节点等元数据。 */
    atomic_ulong clock;              /* 为每次访问生成使用序号。 */
    int readcnt;                    /* 当前读者数量，初始化为 0。 */
    sem_t mutex;                    /* 初始化为 1，只保护 readcnt。 */
    sem_t w;                        /* 初始化为 1，控制缓存的读写访问。 */
} cache_t;

static cache_t cache;

/* 等待信号量；被信号中断时重新等待，对应教材中的 P 操作。 */
static int cache_wait(sem_t *sem)
{
    int result;
    do {
        result = sem_wait(sem); /* P */
    } while (result < 0 && errno == EINTR);
    return result;
}

/* 插入已分配好的节点；调用方须持有 w，并确保容量足够且没有重复 key。 */
static void cache_insert(cache_entry_t *entry)
{
    unsigned long sequence = atomic_fetch_add(&cache.clock, 1) + 1;
    atomic_init(&entry->last_used, sequence);
    entry->next = cache.head;
    cache.head = entry;
    cache.total_size += entry->size;
}

/* 淘汰最近最少使用的节点；调用方须持有 w。淘汰成功返回 1，空链表返回 0。 */
static int cache_evict(void)
{
    if (cache.head == NULL) {
        return 0;
    }

    /* 1. 找到使用序号最小的节点，并记录它的前驱。 */
    cache_entry_t *oldest = cache.head;
    cache_entry_t *oldest_prev = NULL;
    cache_entry_t *prev = cache.head;
    for (cache_entry_t *entry = cache.head->next; entry != NULL;
         prev = entry, entry = entry->next) {
        if (atomic_load(&entry->last_used) < atomic_load(&oldest->last_used)) {
            oldest = entry;
            oldest_prev = prev;
        }
    }

    /* 2. 从链表摘除节点；被淘汰的节点可能正好是头节点。 */
    if (oldest_prev == NULL) {
        cache.head = oldest->next;
    } else {
        oldest_prev->next = oldest->next;
    }

    /* 3. 更新缓存字节数，释放节点持有的数据和节点本身。 */
    cache.total_size -= oldest->size;
    free(oldest->key);
    free(oldest->data);
    free(oldest);
    return 1;
}

int cache_init(void)
{
    cache.head = NULL;
    cache.total_size = 0;
    cache.readcnt = 0;
    atomic_init(&cache.clock, 0);
    if (sem_init(&cache.mutex, 0, 1) < 0) {
        return -1;
    }
    if (sem_init(&cache.w, 0, 1) < 0) {
        sem_destroy(&cache.mutex);
        return -1;
    }
    return 0;
}

/* 读者：在 mutex 保护下增加 readcnt，第一个读者获取 w；
 *       读取结束后在 mutex 保护下减少 readcnt，最后一个读者释放 w。
 * 写者：获取 w 后修改链表和 total_size，完成后释放 w。
 * 并发读者仍可能更新使用序号，因此 clock 和 last_used 保留为原子变量。
 */

int cache_get(const char *key, unsigned char **data, size_t *size)
{
    /* 1. 将输出初始化为空；检查 key、data、size 是否有效。 */
    if (data != NULL) {
        *data = NULL;
    }
    if (size != NULL) {
        *size = 0;
    }
    if (key == NULL || data == NULL || size == NULL) {
        return -1;
    }
    /* 2. 进入读者区：P(mutex)，增加 readcnt；
     *    若是第一个读者则 P(w)，阻止写者修改链表，最后 V(mutex)。
     */
    if (cache_wait(&cache.mutex) < 0) { /* P */
        return -1;
    }
    ++cache.readcnt;
    if (cache.readcnt == 1 && cache_wait(&cache.w) < 0) { /* P：仅第一个读者执行 */
        --cache.readcnt;
        sem_post(&cache.mutex); /* V */
        return -1;
    }
    sem_post(&cache.mutex); /* V */

    /* 3. 遍历链表，比较节点的 key；未找到则准备返回 0。 */
    int result = 0;
    cache_entry_t *entry = cache.head;
    while (entry != NULL && strcmp(entry->key, key) != 0) {
        entry = entry->next;
    }

    /* 4. 命中时分配响应副本，按节点 size 复制二进制数据。
     *    复制必须在读者区内完成，不能把节点内部指针直接交给调用方。
     *    分配失败返回 -1，输出保持为空。
     */
    if (entry != NULL) {
        unsigned char *copy = malloc(entry->size == 0 ? 1 : entry->size);
        if (copy == NULL) {
            result = -1;
        } else {
            memcpy(copy, entry->data, entry->size);
            /* 5. 原子更新使用序号，返回独立副本及长度。 */
            unsigned long sequence = atomic_fetch_add(&cache.clock, 1) + 1;
            atomic_store(&entry->last_used, sequence);
            *data = copy;
            *size = entry->size;
            result = 1;
        }
    }

    /* 6. 离开读者区：P(mutex)，减少 readcnt；
     *    若是最后一个读者则 V(w)，允许写者进入，最后 V(mutex)。
     *    命中、未命中及分配失败的路径都必须完成退出操作。
     *    释放读者访问权限后，调用方才向客户端发送副本并 free。
     */
    if (cache_wait(&cache.mutex) < 0) { /* P */
        free(*data);
        *data = NULL;
        *size = 0;
        return -1;
    }
    --cache.readcnt;
    if (cache.readcnt == 0) {
        sem_post(&cache.w); /* V：仅最后一个读者执行 */
    }
    sem_post(&cache.mutex); /* V */
    return result;
}

int cache_put(const char *key, const unsigned char *data, size_t size)
{
    /* 1. 检查参数，非法输入返回 -1；超过 MAX_OBJECT_SIZE 时返回 0。 */
    if (key == NULL || data == NULL) {
        return -1;
    }
    if (size > MAX_OBJECT_SIZE) {
        return 0;
    }

    /* 2. 分配新节点，复制 key 和 size 字节的响应数据。
     *    在加锁前完成分配；失败时释放已分配的内存并返回 -1，保留原缓存。
     */
    cache_entry_t *entry = malloc(sizeof(*entry));
    if (entry == NULL) {
        return -1;
    }
    entry->key = strdup(key);
    entry->data = malloc(size == 0 ? 1 : size);
    if (entry->key == NULL || entry->data == NULL) {
        goto fail;
    }
    memcpy(entry->data, data, size);
    entry->size = size;

    /* 3. P(w) 进入写者区，独占链表；检查是否已存在相同 key。
     *    同 key 只保留一份，替换旧节点时扣除其 size 并释放节点及数据。
     */
    if (cache_wait(&cache.w) < 0) { /* P */
        goto fail;
    }
    /* link 指向头指针或前驱节点的 next，便于统一删除头节点和中间节点。 */
    cache_entry_t **link = &cache.head;
    while (*link != NULL && strcmp((*link)->key, key) != 0) {
        link = &(*link)->next;
    }
    if (*link != NULL) {
        cache_entry_t *old = *link;
        *link = old->next;
        cache.total_size -= old->size;
        free(old->key);
        free(old->data);
        free(old);
    }

    /* 4. 若 total_size + size 超过 MAX_CACHE_SIZE，反复调用 cache_evict()，
     *    淘汰最近最少使用的节点，直到剩余空间足够容纳新对象。
     */
    while (size > MAX_CACHE_SIZE - cache.total_size) {
        cache_evict();
    }

    /* 5. 调用 cache_insert(entry)，统一更新使用序号、链表和 total_size。
     */
    cache_insert(entry);

    /* 6. V(w) 离开写者区，成功返回 1。
     *    获取 w 后的所有退出路径都必须释放它；缓存不接管调用方原始数据。
     */
    sem_post(&cache.w); /* V */
    return 1;

fail:
    free(entry->key);
    free(entry->data);
    free(entry);
    return -1;
}
