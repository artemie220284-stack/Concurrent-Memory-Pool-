//
// Created by a2006 on 24-7-20.
//

//#ifndef PAGECACHE_H
//#define PAGECACHE_H

//#endif //PAGECACHE_H
#ifndef PAGE_CACHE_H
#define PAGE_CACHE_H

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>  // 为了使用 sysconf(_SC_PAGESIZE)

#ifndef PAGE_SIZE
#define PAGE_SIZE sysconf(_SC_PAGESIZE)
#endif
typedef struct Page {
    void *data;         // 指向页数据的指针
    bool is_used;       // 标记页是否被使用
    struct Page *next;  // 指向下一个页的指针
} Page;

typedef struct PageCache {
    Page *pages;        // 指向页缓存的指针
    size_t num_pages;   // 页缓存中的页数量
    pthread_mutex_t lock; // 互斥锁，用于同步访问
} PageCache;

// 初始化页缓存
bool PageCache_Init(PageCache *cache, size_t num_pages);

// 从页缓存分配一个页
Page* PageCache_Alloc(PageCache *cache);

// 释放一个页回页缓存
void PageCache_Free(PageCache *cache, Page *page);

// 销毁页缓存
void PageCache_Destroy(PageCache *cache);

#endif // PAGE_CACHE_H
