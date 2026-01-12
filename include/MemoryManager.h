//
// Created by a2006 on 24-7-21.
//

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#define PAGE_SIZE sysconf(_SC_PAGESIZE)
#include <PageCache.h>
// 内存管理器接口
typedef struct PageCacheInternal PageCacheInternal;
typedef struct MemoryManager {
    void* (*alloc)(struct MemoryManager*, size_t);
    void (*free)(struct MemoryManager*, void*);
    void (*destroy)(struct MemoryManager *mm);
    PageCacheInternal *internal;
} MemoryManager;
extern MemoryManager g_memory_manager;
// 初始化内存管理器
bool MemoryManager_Init(MemoryManager *mm);
void MemoryManager_Destroy(MemoryManager *mm);
void* MemoryManager_AllocPage(struct MemoryManager *mm, size_t size);
void MemoryManager_FreePage(struct MemoryManager *mm, void *ptr);
extern MemoryManager g_memory_manager;
#endif // MEMORY_MANAGER_H
