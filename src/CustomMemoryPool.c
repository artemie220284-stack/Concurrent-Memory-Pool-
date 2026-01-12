//
// Created by a2006 on 24-7-24.
//
#include <stdlib.h>
#include <pthread.h>

#define POOL_SIZE 1024 * 1024  // 1MB

typedef struct {
    char* memory;
    size_t used;
    pthread_mutex_t lock;
} MemoryPool;

MemoryPool* create_memory_pool() {
    MemoryPool* pool = malloc(sizeof(MemoryPool));
    pool->memory = malloc(POOL_SIZE);
    pool->used = 0;
    pthread_mutex_init(&pool->lock, NULL);
    return pool;
}

void* pool_alloc(MemoryPool* pool, size_t size) {
    pthread_mutex_lock(&pool->lock);
    if (pool->used + size > POOL_SIZE) {
        pthread_mutex_unlock(&pool->lock);
        return NULL;
    }
    void* ptr = pool->memory + pool->used;
    pool->used += size;
    pthread_mutex_unlock(&pool->lock);
    return ptr;
}

void destroy_memory_pool(MemoryPool* pool) {
    free(pool->memory);
    pthread_mutex_destroy(&pool->lock);
    free(pool);
}