// UnLocked.h
#ifndef UNLOCKED_H
#define UNLOCKED_H

#include <stdatomic.h>
#include <stddef.h>



#define CACHE_LINE_SIZE 64
#define NUM_SIZE_CLASSES 4
#define NUM_BLOCKS 1048576


    typedef struct {
        atomic_int next;
        int size_class;
        char padding[CACHE_LINE_SIZE - sizeof(atomic_int) - sizeof(int)];
    } Block;


typedef struct {
    Block* blocks;
    atomic_int free_list;
    size_t block_size;
    size_t num_blocks;
    char padding[CACHE_LINE_SIZE - 3 * sizeof(size_t) - sizeof(atomic_int)];
} SizeClass;

typedef struct {
    SizeClass size_classes[NUM_SIZE_CLASSES];
} Allocator;

void init_allocator(Allocator* a);
void* alloc(Allocator* a, size_t size);
void custom_free(Allocator* a, void* ptr);

#endif // UNLOCKED_H