#include "CentralCache.h"
#include <pthread.h>
#include <stdlib.h>

#define NUM_SIZE_CLASSES 32
#define MIN_BLOCK_SIZE 8
#define MAX_BLOCK_SIZE 8192



static int get_size_class(size_t size) {
    // 实现 size class 的选择逻辑
    return 0;  // 临时返回值，需要根据实际情况实现
}
int CentralCache_BatchAlloc(CentralCache *cache, void **blocks, int num_blocks);
static void* pop_from_free_list(void** free_list) {
    if (*free_list == NULL) return NULL;
    void* result = *free_list;
    *free_list = *((void**)*free_list);
    return result;
}

static void push_to_free_list(void** free_list, void* ptr) {
    *((void**)ptr) = *free_list;
    *free_list = ptr;
}

bool CentralCache_Init(CentralCache *cache, size_t chunk_size) {
    if (!cache || chunk_size == 0) return false;

    cache->chunks = NULL;
    cache->chunk_size = chunk_size;
    cache->num_chunks = 0;

    if (pthread_mutex_init(&cache->lock, NULL) != 0) {
        return false;
    }

    return true;
}

void* CentralCache_Alloc(CentralCache *cache, size_t size) {
    if (!cache || size == 0) return NULL;

    pthread_mutex_lock(&cache->lock);

    void* result = NULL;
    if (cache->num_chunks > 0) {
        result = pop_from_free_list(&cache->chunks);
        if (result) {
            cache->num_chunks--;
        }
    }

    if (!result) {
        // 如果没有可用的块，分配新的chunk
        void* new_chunk = malloc(cache->chunk_size);
        if (new_chunk) {
            result = new_chunk;
            // 将剩余的空间添加到空闲列表
            size_t remaining = cache->chunk_size - size;
            char* ptr = (char*)new_chunk + size;
            while (remaining >= size) {
                push_to_free_list(&cache->chunks, ptr);
                ptr += size;
                remaining -= size;
                cache->num_chunks++;
            }
        }
    }

    pthread_mutex_unlock(&cache->lock);
    return result;
}

void CentralCache_Free(CentralCache *cache, void *ptr) {
    if (!cache || !ptr) return;

    pthread_mutex_lock(&cache->lock);

    push_to_free_list(&cache->chunks, ptr);
    cache->num_chunks++;

    pthread_mutex_unlock(&cache->lock);
}

void CentralCache_Destroy(CentralCache *cache) {
    if (!cache) return;

    pthread_mutex_lock(&cache->lock);

    // 释放所有分配的内存
    while (cache->chunks) {
        void* chunk = pop_from_free_list(&cache->chunks);
        free(chunk);
    }

    pthread_mutex_unlock(&cache->lock);
    pthread_mutex_destroy(&cache->lock);
}
int CentralCache_BatchAlloc(CentralCache *cache, void **blocks, int num_blocks) {
    if (!cache || !blocks || num_blocks <= 0) return 0;

    pthread_mutex_lock(&cache->lock);

    int allocated = 0;
    for (int i = 0; i < num_blocks && cache->num_chunks > 0; i++) {
        blocks[i] = pop_from_free_list(&cache->chunks);
        if (blocks[i]) {
            allocated++;
            cache->num_chunks--;
        } else {
            break;
        }
    }

    pthread_mutex_unlock(&cache->lock);
    return allocated;
}