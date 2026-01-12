
#include "UnLocked.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

static size_t size_classes[NUM_SIZE_CLASSES] = {16, 32, 64, 128};

void init_allocator(Allocator* a) {
    for (int i = 0; i < NUM_SIZE_CLASSES; i++) {
        SizeClass* sc = &a->size_classes[i];
        sc->block_size = size_classes[i];
        sc->num_blocks = NUM_BLOCKS;
        sc->blocks = aligned_alloc(CACHE_LINE_SIZE, NUM_BLOCKS * sizeof(Block));
        if (sc->blocks == NULL) {
            fprintf(stderr, "Failed to allocate memory for size class %d\n", i);
            exit(1);
        }
        memset(sc->blocks, 0, NUM_BLOCKS * sizeof(Block));

        for (int j = 0; j < NUM_BLOCKS - 1; j++) {
            atomic_store(&sc->blocks[j].next, j + 1);
            sc->blocks[j].size_class = i;
        }
        atomic_store(&sc->blocks[NUM_BLOCKS - 1].next, -1);
        sc->blocks[NUM_BLOCKS - 1].size_class = i;
        atomic_store(&sc->free_list, 0);
    }
}

static inline int get_size_class(size_t size) {
    for (int i = 0; i < NUM_SIZE_CLASSES; i++) {
        if (size <= size_classes[i]) {
            return i;
        }
    }
    return -1;
}

void* alloc(Allocator* a, size_t size) {
    int sc_index = get_size_class(size);
    if (sc_index == -1) {
        fprintf(stderr, "Requested size %zu is too large\n", size);
        return NULL;
    }

    SizeClass* sc = &a->size_classes[sc_index];
    while (1) {
        int index = atomic_load(&sc->free_list);
        if (index == -1) {
            fprintf(stderr, "Out of memory for size class %d\n", sc_index);
            return NULL;
        }
        int next = atomic_load(&sc->blocks[index].next);
        if (atomic_compare_exchange_weak(&sc->free_list, &index, next)) {
            return &sc->blocks[index];
        }
    }
}

void custom_free(Allocator* a, void* ptr) {
    Block* block = (Block*)ptr;
    SizeClass* sc = &a->size_classes[block->size_class];
    intptr_t index = (block - sc->blocks);

    while (1) {
        int old_head = atomic_load(&sc->free_list);
        atomic_store(&block->next, old_head);
        if (atomic_compare_exchange_weak(&sc->free_list, &old_head, index)) {
            return;
        }
    }
}