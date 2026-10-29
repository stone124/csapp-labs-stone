#include "cachelab.h"
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

// 首先把程序当作黑盒
// 输入：cachelab/traces/yi.trace
// 输入2: 命令行参数
// 详细模式输出:
// L 10,1 miss
// M 20,1 miss hit
// L 22,1 hit
// S 18,1 hit
// L 110,1 miss eviction
// L 210,1 miss eviction
// M 12,1 miss eviction hit
// 输出：cachelab/cachelab.h 中的 printSummary 函数

/* 访存结果字符串。 */
static const char HIT[] = "hit";
static const char MISS[] = "miss";
static const char EVICTION[] = "eviction";

/* 地址各部分在 address_parts 数组中的下标。 */
enum address_part_index {
    TAG_INDEX = 0,
    SET_INDEX = 1,
    BLOCK_OFFSET_INDEX = 2
};

/* result 数组中各统计项的下标。 */
enum cache_stat_index {
    HIT_COUNT_INDEX = 0,
    MISS_COUNT_INDEX = 1,
    EVICTION_COUNT_INDEX = 2
};

/* ==================== 结构体 ==================== */

struct cli_options
{
    bool h;
    bool v;
    int s;
    int E;
    int b;
    char *t;
};

/* 缓存行。 */
struct cache_line {
    bool valid;                     // 有效位
    unsigned long long tag;          // 标记
    unsigned long long last_used;    // 最近访问序号，用于 LRU
};

/* 缓存组。 */
struct cache_set {
    struct cache_line *lines;        // E 条缓存行
};

/* 整个缓存。 */
struct cache {
    int s;
    int E;
    int b; // block大小
    struct cache_set *sets;          // 2^s 个缓存组,set[]访问对应组
    unsigned long long clock;        // 全局访问计数器
};

/* 组内查找结果，下标为 -1 表示未找到。 */
struct line_search_result {
    int hit_index;
    int first_invalid_index;
    int lru_index;
};

/* ==================== 函数声明 ==================== */

/* 解析命令行参数。 */
struct cli_options handle_argc(int argc, char *argv[]);

/* 执行模拟，将统计写入 result。 */
void simulate_cache(struct cli_options *options, int result[3]);

/* 根据 h 选项决定是否打印帮助信息。 */
void handle_h(bool h);

/* 打开 trace 文件。 */
FILE *open_trace(const char *trace_path);

/* 模拟访存并统计结果，按需打印详细信息。 */
void run_cache_simulation(int s, int E, int b, bool verbose,
                          FILE *trace, int result[3]);

/* 分配并初始化缓存。 */
struct cache *create_cache(int s, int E, int b);

/* 读取下一条有效记录，跳过 I；文件结束时返回 false。 */
bool read_trace_record(FILE *trace, char *op,
                       unsigned long long *address, int *size);

/* 处理一次访存，更新统计，并按 verbose 打印本次结果。 */
void process_access(struct cache *cache, unsigned long long address,
                    bool verbose, int result[3]);

/* 在组内查找命中行、首个空行及最近最少使用的有效行。 */
struct line_search_result find_line(
    const struct cache_set *set, int E, unsigned long long tag);

/* 将地址拆分结果写入 address_parts。 */
void abstract_tag_set_line_from_address(unsigned long long address, const int s,
                                      const int b, unsigned long long address_parts[3]);

/* 释放缓存内存。 */
void destroy_cache(struct cache *cache);

/* ==================== 主函数 ==================== */

int main(int argc, char *argv[])
{
    // 1.处理命令行参数
    struct cli_options options = handle_argc(argc, argv);

    // 2.计算命中、未命中、替换次数，按命令行详细打印参数进行逻辑处理
    int result[3] = {0};
    simulate_cache(&options, result);

    // 3.汇总打印
    printSummary(result[HIT_COUNT_INDEX], result[MISS_COUNT_INDEX], result[EVICTION_COUNT_INDEX]);
    return 0;
}

/* ==================== 1. 处理命令行参数 ==================== */

/**
 * @brief 读取 命令行参数，解析出-h, -v, -s, -E, -b, -t 参数，并判断合法
 * @param int argc
 * @param char *argv[]
 *
 * @return 返回参数结构体对应的格式 cli_options
 */
struct cli_options handle_argc(int argc, char *argv[]){
    int opt;
    struct cli_options parse_temp = {
        .h = false,
        .v = false,
        .s = 0,
        .E = 0,
        .b = 0,
        .t = NULL
    };
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1){
        switch (opt) {
            case 'h':
                parse_temp.h = true;
                break;
            case 'v':
                parse_temp.v = true;
                break;
            case 's':
                parse_temp.s = atoi(optarg);
                break;
            case 'E':
                parse_temp.E = atoi(optarg);
                break;
            case 'b':
                parse_temp.b = atoi(optarg);
                break;
            case 't':
                parse_temp.t = optarg;
                break;
            default:
                fprintf(stderr, "错误：无效选项或缺少选项参数，请使用 -h 查看帮助。\n");
                exit(EXIT_FAILURE);
        }
    }
    return parse_temp;
}

/* ==================== 2. 模拟缓存访问 ==================== */

/**
 * @brief 按cli_options的对应参数进行处理
 *
 * @param struct cli_options* options解析的命令行参数
 *
 * @param result 输出统计：下标 0 为命中，1 为未命中，2 为驱逐次数。
 */
void simulate_cache(struct cli_options *options, int result[3]){
    // 1 处理h
    handle_h(options->h);
    // 2.读取trace路径内容
    FILE *trace = open_trace(options->t);
    // 3.按照-s, -E, -b, trace内容进行模拟
    run_cache_simulation(options->s, options->E, options->b, options->v, trace, result);
    // 4.关闭trace文件
    fclose(trace);
}

/* 根据 h 选项决定是否打印帮助信息。 */
void handle_h(bool h){
    // TODO
    return;
}

/* 以只读模式打开 trace 文件，失败时报告错误并退出。 */
FILE *open_trace(const char *trace_path)
{
    if (trace_path == NULL) {
        fprintf(stderr, "错误：未指定 trace 文件路径。\n");
        exit(EXIT_FAILURE);
    }

    FILE *trace = fopen(trace_path, "r");
    if (trace == NULL) {
        perror(trace_path);
        exit(EXIT_FAILURE);
    }

    return trace;
}

/* 根据 s、E、b 模拟缓存访问，按 verbose 控制详细输出，并写入统计结果。 */
void run_cache_simulation(int s, int E, int b, bool verbose, FILE *trace, int result[3]){
    // 1.创建缓存并清零统计
    struct cache *cache = create_cache(s, E, b);
    result[HIT_COUNT_INDEX] = result[MISS_COUNT_INDEX] = result[EVICTION_COUNT_INDEX] = 0;

    // 2.逐条读取有效访存记录
    char op;
    unsigned long long address;
    int size;
    while (read_trace_record(trace, &op, &address, &size)) {
        // 3.模拟访问，M 操作访问两次；按需打印详细信息
        if (verbose) {
            printf("%c %llx,%d", op, address, size);
        }
        process_access(cache, address, verbose, result);
        if (op == 'M') {
            process_access(cache, address, verbose, result);
        }
        if (verbose) {
            printf("\n");
        }
    }

    // 4.释放缓存
    destroy_cache(cache);
}

/* 分配并初始化缓存。 */
struct cache *create_cache(int s, int E, int b)
{
    struct cache *new_cache = malloc(sizeof(struct cache));
    if (new_cache == NULL) {
        fprintf(stderr, "错误：分配缓存失败。\n");
        exit(EXIT_FAILURE);
    }
    // 1.赋值对应s,E,b值
    new_cache->s = s;
    new_cache->E = E;
    new_cache->b = b;
    new_cache->clock = 0;

    // 2.malloc对应的cache set
    size_t set_count = (size_t)1 << s;// 计算2^s
    new_cache->sets = malloc(set_count * sizeof(struct cache_set));
    if (new_cache->sets == NULL) {
        fprintf(stderr, "错误：分配缓存组失败。\n");
        free(new_cache);
        exit(EXIT_FAILURE);
    }
    // 3.为每个组分配并初始化缓存行
    for (size_t set = 0; set < set_count; set++) {
        new_cache->sets[set].lines = malloc(E * sizeof(struct cache_line));
        if (new_cache->sets[set].lines == NULL) {
            fprintf(stderr, "错误：分配缓存行失败。\n");
            for (size_t previous = 0; previous < set; previous++) {
                free(new_cache->sets[previous].lines);
            }
            free(new_cache->sets);
            free(new_cache);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < E; i++) {
            new_cache->sets[set].lines[i] = (struct cache_line){0};
        }
    }

    return new_cache;
}

/* 读取下一条有效记录，跳过 I；文件结束时返回 false。 */
bool read_trace_record(FILE *trace, char *op,
                       unsigned long long *address, int *size)
{
    // 1.逐行读取并解析记录
    char line[256];
    char parsed_op;
    unsigned long long parsed_address;
    int parsed_size;
    while (fgets(line, sizeof line, trace) != NULL) {
        if (sscanf(line, " %c %llx,%d", &parsed_op,
                   &parsed_address, &parsed_size) != 3) {
            continue;
        }

        // 2.仅保留 L、S、M 操作
        if (parsed_op != 'L' && parsed_op != 'S' && parsed_op != 'M') {
            continue;
        }

        // 3.写入记录并返回成功
        *op = parsed_op;
        *address = parsed_address;
        *size = parsed_size;
        return true;
    }
    return false;
}

/* 处理一次访存，更新统计，并按 verbose 打印本次结果。 */
void process_access(struct cache *cache, unsigned long long address,
                    bool verbose, int result[3])
{
    // 1.解析地址中的 tag 和 set 索引
    unsigned long long address_parts[3] = {0};
    abstract_tag_set_line_from_address(address, cache->s, cache->b, address_parts);

    // 2.定位缓存组
    struct cache_set *set = &cache->sets[address_parts[SET_INDEX]];

    // 3.查找命中行、空行及 LRU 候选
    struct line_search_result line_result = find_line(set, cache->E, address_parts[TAG_INDEX]);

    // 4.更新统计、缓存行及 LRU 信息，按需打印结果
    int type = line_result.hit_index != -1 ? 0 :
               line_result.first_invalid_index != -1 ? 1 : 2;
    int index = line_result.hit_index;
    switch (type) {
    case 0:
        result[HIT_COUNT_INDEX]++;
        if (verbose) printf(" %s", HIT);
        break;
    case 1:
        index = line_result.first_invalid_index;
        result[MISS_COUNT_INDEX]++;
        if (verbose) printf(" %s", MISS);
        break;
    case 2:
        index = line_result.lru_index;
        result[MISS_COUNT_INDEX]++;
        result[EVICTION_COUNT_INDEX]++;
        if (verbose) printf(" %s %s", MISS, EVICTION);
        break;
    }

    set->lines[index].valid = true;
    set->lines[index].tag = address_parts[TAG_INDEX];
    set->lines[index].last_used = ++cache->clock;
    return;
}

/* 在组内查找命中行、首个空行及最近最少使用的有效行。 */
struct line_search_result find_line(
    const struct cache_set *set, int E, unsigned long long tag)
{
    // -1 表示尚未找到对应的行
    struct line_search_result result = {
        .hit_index = -1,
        .first_invalid_index = -1,
        .lru_index = -1
    };

    for (int i = 0; i < E; i++) {
        const struct cache_line *line = &set->lines[i];

        if (line->valid) {
            // 有效行：先检查 tag，匹配则命中，无需继续查找
            if (line->tag == tag) {
                result.hit_index = i;
                return result;
            }

            // tag 不匹配：将该行纳入 LRU 候选
            // 尚无候选，或该行比原候选更久未使用时，更新候选下标
            if (result.lru_index == -1 ||
                line->last_used < set->lines[result.lru_index].last_used) {
                result.lru_index = i;
            }
        } else if (result.first_invalid_index == -1) {
            // 无效行：只记录遇到的第一个空行，供未命中时优先使用
            result.first_invalid_index = i;
        }
    }

    // 未命中：调用方优先使用空行，没有空行时才使用 LRU 候选
    return result;
}

/* 释放缓存内存。 */
void destroy_cache(struct cache *cache)
{
    if (cache == NULL) {
        return;
    }

    // 1.释放每组的缓存行
    size_t set_count = (size_t)1 << cache->s;
    for (size_t i = 0; i < set_count; i++) {
        free(cache->sets[i].lines);
    }

    // 2.释放组数组
    free(cache->sets);

    // 3.释放缓存结构体
    free(cache);
}


/* 提取 tag、set 索引和块偏移，写入调用方提供的数组。 */
void abstract_tag_set_line_from_address(unsigned long long address, const int s,
                                      const int b, unsigned long long address_parts[3])
{
    address_parts[TAG_INDEX] = address >> (s + b);
    address_parts[SET_INDEX] = (address >> b) & ((1ULL << s) - 1);
    address_parts[BLOCK_OFFSET_INDEX] = address & ((1ULL << b) - 1);
}



/* ==================== 3. 汇总打印 ==================== */
/* main 中调用 printSummary，实现在 cachelab.c 中。 */
