//
// Created by a2006 on 24-7-20.
//


#ifndef CENTRAL_CACHE_H
#define CENTRAL_CACHE_H

#include "ThreadCache.h"
#include <stdlib.h>
#include <stdbool.h>

#include <pthread.h>
#define CENTRAL_CACHE_CHUNK_SIZE (1024 * 1024) // 假设中心缓存块大小为1MB
#include <sys/mman.h>
#include <unistd.h>  // 为了使用 sysconf(_SC_PAGESIZE)

#ifndef PAGE_SIZE
#define PAGE_SIZE sysconf(_SC_PAGESIZE)
#endif
typedef struct CentralCache {
    void**chunks; // 指向所有内存块的指针数组
    size_t chunk_size; // 每个内存块的大小
    size_t num_chunks; // 当前拥有的内存块数量
    pthread_mutex_t lock; // 互斥锁，用于同步访问
} CentralCache;
int CentralCache_BatchAlloc(CentralCache *cache, void **blocks, int num_blocks);
// 初始化中心缓存
bool CentralCache_Init(CentralCache *cache, size_t chunk_size);

// 从中心缓存分配内存块给线程缓存
void* CentralCache_Alloc(CentralCache *cache, size_t size);

// 将线程缓存的内存块回收给中心缓存
void CentralCache_Free(CentralCache *cache, void *ptr);

// 销毁中心缓存
void CentralCache_Destroy(CentralCache *cache);

#endif // CENTRAL_CACHE_H

