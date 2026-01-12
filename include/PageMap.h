//
// Created by a2006 on 24-7-20.
//
// PageMap.h
#ifndef PAGE_MAP_H
#define PAGE_MAP_H

#include <stdint.h>
#include <stdbool.h>

// 定义页映射结构
typedef struct {
    void* start_address;  // 映射区域的起始地址
    size_t size;          // 映射区域的大小
    uint32_t* bitmap;     // 位图，用于跟踪页的使用情况
    size_t bitmap_size;   // 位图的大小（以字节为单位）
} PageMap;

// 初始化页映射
bool PageMap_Init(PageMap* map, void* start_address, size_t size);

// 在页映射中分配一个页
void* PageMap_AllocatePage(PageMap* map);

// 在页映射中释放一个页
void PageMap_FreePage(PageMap* map, void* page);

// 检查一个地址是否在页映射中
bool PageMap_Contains(PageMap* map, void* address);

// 销毁页映射
void PageMap_Destroy(PageMap* map);

#endif // PAGE_MAP_H