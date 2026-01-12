//
// Created by a2006 on 24-7-20.
//
#include "ThreadCache.h"
#include <stdlib.h>

#define LOCAL_CACHE_SIZE 1024

typedef struct {
    void* blocks[LOCAL_CACHE_SIZE];
    int count;
} LocalCache;

static __thread LocalCache local_cache = {0};

void* ThreadCache_Get(ThreadCache *cache) {
    if (local_cache.count > 0) {
        return local_cache.blocks[--local_cache.count];
    }

    // 如果本地缓存为空，从中心缓存批量获取
    void* blocks[LOCAL_CACHE_SIZE];
    int obtained = CentralCache_BatchAlloc(cache->central_cache, blocks, LOCAL_CACHE_SIZE);

    if (obtained > 0) {
        local_cache.count = obtained - 1;
        for (int i = 0; i < local_cache.count; i++) {
            local_cache.blocks[i] = blocks[i + 1];
        }
        return blocks[0];
    }

    return NULL;
}

void ThreadCache_Free(ThreadCache *cache, void* ptr) {
    if (local_cache.count < LOCAL_CACHE_SIZE) {
        local_cache.blocks[local_cache.count++] = ptr;
    } else {
        CentralCache_Free(cache->central_cache, ptr);
    }
}