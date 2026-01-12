//
// Created by a2006 on 24-7-24.
//
// concurrent_memory_pool.h

#ifndef CONCURRENT_MEMORY_POOL_H
#define CONCURRENT_MEMORY_POOL_H

#include <stddef.h>

// Function to allocate memory
void* hp_alloc(size_t size);

// Function to free memory
void hp_free(void* ptr, size_t size);

#endif // CONCURRENT_MEMORY_POOL_H
