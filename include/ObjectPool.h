//
// Created by a2006 on 24-7-20.
//




#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <stdlib.h>
#include <stdbool.h>
#include "MemoryManager.h"

#define POOL_DEFAULT_CHUNK_SIZE 1024

typedef struct ObjectPool {
    void *pool;             // 内存池的起始地址
    size_t object_size;     // 每个对象的大小
    size_t chunk_size;      // 内存池每次扩展的大小
    size_t current_position; // 当前分配到的位置
    MemoryManager mm;
    struct ObjectPool *next; // 指向下一个内存池块的指针
} ObjectPool;

// 初始化内存池
bool ObjectPool_Init(ObjectPool **pool, size_t object_size, size_t chunk_size);

// 从内存池中分配一个对象
void* ObjectPool_Alloc(ObjectPool **pool);

// 释放整个内存池
void ObjectPool_Destroy(ObjectPool **pool);




#endif // OBJECTPOOL_H


