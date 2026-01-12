#ifndef CONCURRENT_ALLOC_H
#define CONCURRENT_ALLOC_H

#include <stddef.h>
#include <stdbool.h>
#include "MemoryManager.h"
// 并发内存分配器接口
void* ConcurrentAlloc(size_t size);
void ConcurrentFree(void *ptr);

// 内存管理器初始化和销毁函数
bool InitializeMemoryManager(void);
void DestroyMemoryManager(void);

#endif // CONCURRENT_ALLOC_H