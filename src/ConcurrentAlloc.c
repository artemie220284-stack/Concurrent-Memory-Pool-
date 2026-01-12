//
// Created by a2006 on 24-7-22.
//
#include "ConcurrentAlloc.h"
#include "MemoryManager.h"
#include <stdlib.h>

// 假设 MemoryManager 是全局的，且已经初始化
extern MemoryManager g_memory_manager;

void* ConcurrentAlloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    return g_memory_manager.alloc(&g_memory_manager, size);
}

void ConcurrentFree(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    g_memory_manager.free(&g_memory_manager, ptr);
}

// 初始化内存管理器，这个函数应该在程序启动时调用一次
bool InitializeMemoryManager() {
    return MemoryManager_Init(&g_memory_manager);
}

// 销毁内存管理器，这个函数应该在程序结束时调用一次
void DestroyMemoryManager() {
    g_memory_manager.destroy(&g_memory_manager);
}
