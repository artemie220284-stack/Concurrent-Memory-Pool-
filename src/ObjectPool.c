//
// Created by a2006 on 24-7-21.
//
#include "ObjectPool.h"
#include <stdlib.h>
#include <pthread.h>
#include "MemoryManager.h"
bool ObjectPool_Init(ObjectPool **pool, size_t object_size, size_t chunk_size) {
    if (!pool || object_size == 0 || chunk_size == 0) {
        return false;
    }

    MemoryManager_Init(&(*pool)->mm);

    (*pool)->object_size = object_size;
    (*pool)->chunk_size = chunk_size;
    (*pool)->current_position = 0;
    (*pool)->next = NULL;

    // 分配第一个内存块
    (*pool)->pool=(*pool)->mm.alloc(&(*pool)->mm,chunk_size);
    if (!(*pool)->pool) {
        free(*pool);
        return false;
    }

    return true;
}

void* ObjectPool_Alloc(ObjectPool **pool) {
    if (!pool || !*pool) {
        return NULL;
    }

    ObjectPool *current_pool = *pool;

   return (*pool)->mm.alloc(&(*pool)->mm,(*pool)->object_size);
}

void ObjectPool_Destroy(ObjectPool **pool) {
    if (!pool || !*pool) {
        return;
    }

    ObjectPool *current = *pool;
    while (current) {
        ObjectPool *next = current->next;
       //销毁内存管理器

        MemoryManager_Destroy(&current->mm);
        free(current);
        current = next;
    }

    *pool = NULL;
}
