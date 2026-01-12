//
// Created by a2006 on 24-7-20.
//

#ifndef THREADCACHE_H
#define THREADCACHE_H
#ifndef THREAD_CACHE_H
#define THREAD_CACHE_H

#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include "CentralCache.h"
#define DEFAULT_CACHE_SIZE 1024
typedef struct ThreadCache {
    void *cache;             // 缓存空间的起始地址
    size_t cache_size;       // 缓存空间的大小
    pthread_key_t thread_key; // 线程特定的键
    struct CentralCache *central_cache;
} ThreadCache;

// 初始化线程缓存
bool ThreadCache_Init(ThreadCache *cache, size_t cache_size);

// 获取当前线程的缓存空间
void* ThreadCache_Get(ThreadCache *cache);

// 销毁线程缓存
void ThreadCache_Destroy(ThreadCache *cache);

#endif // THREAD_CACHE_H

#endif //THREADCACHE_H
