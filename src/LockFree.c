//
// Created by a2006 on 24-7-24.
//
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <pthread.h>
#include "LockFree.h"
#define CACHE_LINE_SIZE 64
#define NUM_SIZE_CLASSES 32
#define MAX_SMALL_SIZE 256
#define PAGE_SIZE 4096
#define ITEMS_PER_BLOCK 32
#include <stdio.h>
typedef struct Block {
    struct Block* next;
    char items[ITEMS_PER_BLOCK][MAX_SMALL_SIZE];
} Block;

typedef struct {
    _Atomic(Block*) head;
    char pad[CACHE_LINE_SIZE - sizeof(_Atomic(Block*))];
} aligned_atomic_ptr;

typedef struct {
    aligned_atomic_ptr free_lists[NUM_SIZE_CLASSES];
} CentralCache;

static __thread Block* thread_cache[NUM_SIZE_CLASSES] = {0};
static CentralCache central_cache = {0};

static inline int size_class(size_t size) {
    return (size + 7) / 8 - 1;
}

#include <errno.h>

static Block* alloc_block() {
    Block* block = aligned_alloc(CACHE_LINE_SIZE, sizeof(Block));
    if (block == NULL) {
        fprintf(stderr, "aligned_alloc failed: %s\n", strerror(errno));
        return NULL;
    }
    memset(block, 0, sizeof(Block));
    return block;
}
static void* alloc_from_block(Block* block, int sc) {
    size_t size = (sc + 1) * 8;
    for (int i = 0; i < ITEMS_PER_BLOCK; i++) {
        if (block->items[i][0] == 0) {
            block->items[i][0] = 1;
            return block->items[i];
        }
    }
    return NULL;
}

void* hp_alloc(size_t size) {
    if (size > MAX_SMALL_SIZE) {
        return malloc(size);
    }

    int sc = size_class(size);

    // Try thread-local cache first
    if (thread_cache[sc]) {
        void* result = alloc_from_block(thread_cache[sc], sc);
        if (result) return result;
    }

    // Refill thread-local cache from central cache
    Block* old_head;
    do {
        old_head = atomic_load(&central_cache.free_lists[sc].head);
        if (!old_head) {
            // Central cache is empty, allocate a new block
            Block* new_block = alloc_block();
            thread_cache[sc] = new_block;
            return alloc_from_block(new_block, sc);
        }
    } while (!atomic_compare_exchange_weak(&central_cache.free_lists[sc].head, &old_head, old_head->next));

    thread_cache[sc] = old_head;
    return alloc_from_block(old_head, sc);
}

void hp_free(void* ptr, size_t size) {
    if (size > MAX_SMALL_SIZE) {
        free(ptr);
        return;
    }

    int sc = size_class(size);
    Block* block = (Block*)((uintptr_t)ptr & ~(PAGE_SIZE - 1));

    // Use atomic operations to mark as free and check block fullness
    int used = ITEMS_PER_BLOCK;
    for (int i = 0; i < ITEMS_PER_BLOCK; i++) {
        if (&block->items[i][0] == ptr) {
            atomic_store((_Atomic char*)&block->items[i][0], 0);
            used--;
        } else if (atomic_load((_Atomic char*)&block->items[i][0]) == 0) {
            used--;
        }
    }

    // If the block is empty, return it to the central cache
    if (used == 0) {
        Block* old_head;
        do {
            old_head = atomic_load(&central_cache.free_lists[sc].head);
            block->next = old_head;
        } while (!atomic_compare_exchange_weak(&central_cache.free_lists[sc].head, &old_head, block));

        // Update thread cache without atomic operation
        thread_cache[sc] = NULL;
    }
}
