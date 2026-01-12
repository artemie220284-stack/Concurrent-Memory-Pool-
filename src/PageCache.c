//
// Created by a2006 on 24-7-20.
//
#include "PageCache.h"

bool PageCache_Init(PageCache *cache, size_t num_pages) {
    if (!cache) {
        return false;
    }

    cache->pages = NULL;
    cache->num_pages = num_pages;

    // 初始化互斥锁
    if (pthread_mutex_init(&cache->lock, NULL) != 0) {
        return false;
    }

    // 分配页
    cache->pages = (Page *)malloc(num_pages * sizeof(Page));
    if (!cache->pages) {
        pthread_mutex_destroy(&cache->lock);
        return false;
    }

    for (size_t i = 0; i < num_pages; ++i) {
        cache->pages[i].data = malloc(PAGE_SIZE);
        if (!cache->pages[i].data) {
            // 如果分配失败，释放已分配的页并退出
            for (size_t j = 0; j < i; ++j) {
                free(cache->pages[j].data);
            }
            free(cache->pages);
            pthread_mutex_destroy(&cache->lock);
            return false;
        }
        cache->pages[i].is_used = false;
        cache->pages[i].next = (i < num_pages - 1) ? &cache->pages[i + 1] : NULL;
    }

    return true;
}

Page* PageCache_Alloc(PageCache *cache) {
    if (!cache) {
        return NULL;
    }

    pthread_mutex_lock(&cache->lock);

    // 查找未使用的页
    for (size_t i = 0; i < cache->num_pages; ++i) {
        if (!cache->pages[i].is_used) {
            cache->pages[i].is_used = true;
            pthread_mutex_unlock(&cache->lock);
            return &cache->pages[i];
        }
    }

    pthread_mutex_unlock(&cache->lock);
    return NULL; // 没有可用页
}

void PageCache_Free(PageCache *cache, Page *page) {
    if (!cache || !page) {
        return;
    }

    pthread_mutex_lock(&cache->lock);

    // 标记页为未使用
    page->is_used = false;

    pthread_mutex_unlock(&cache->lock);
}

void PageCache_Destroy(PageCache *cache) {
    if (!cache) {
        return;
    }

    pthread_mutex_lock(&cache->lock);

    // 释放所有页
    for (size_t i = 0; i < cache->num_pages; ++i) {
        free(cache->pages[i].data);
    }
    free(cache->pages);

    cache->pages = NULL;
    cache->num_pages = 0;

    pthread_mutex_unlock(&cache->lock);
    pthread_mutex_destroy(&cache->lock);
}
