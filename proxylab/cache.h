#ifndef PROXY_CACHE_H
#define PROXY_CACHE_H

#include <stddef.h>

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* 启动时调用一次，在创建工作线程之前初始化；成功返回 0，失败返回 -1。 */
int cache_init(void);

/* 查询缓存，key 由调用方根据 host、port、path 构造。
 * 命中返回 1，并通过 data、size 返回响应副本及字节数；调用方负责 free。
 * 未命中返回 0，失败返回 -1；这两种情况下 data 为 NULL，size 为 0。
 */
int cache_get(const char *key, unsigned char **data, size_t *size);

/* 写入完整响应，模块内部复制 key 和 data，调用方仍负责原始数据。
 * 成功返回 1，对象过大而跳过时返回 0，失败返回 -1。
 * 空间不足时淘汰最近最少使用的对象。
 */
int cache_put(const char *key, const unsigned char *data, size_t size);

#endif
