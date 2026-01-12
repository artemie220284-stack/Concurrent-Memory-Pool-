//
// Created by a2006 on 24-7-21.
//
#include "ConcurrentAlloc.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "MemoryManager.h"
#include <string.h>
#include "PageCache.h"
#ifndef PAGE_SIZE
#define PAGE_SIZE sysconf(_SC_PAGESIZE)
#endif

typedef struct PageCacheInternal {
    pthread_mutex_t lock;
    size_t num_pages;
    void **pages;
    size_t *page_sizes;  // Array to store the size of each allocated page
    PageCache page_cache;  // Add PageCache as a member
} PageCacheInternal;




MemoryManager g_memory_manager = {0};

bool MemoryManager_Init(MemoryManager *mm) {
    if (!mm) return false;

    PageCacheInternal *internal = calloc(1, sizeof(PageCacheInternal));
    if (!internal) return false;

    if (pthread_mutex_init(&internal->lock, NULL) != 0) {
        free(internal);
        return false;
    }

    mm->alloc = MemoryManager_AllocPage;
    mm->free = MemoryManager_FreePage;
    mm->internal = internal;

    return true;
}
void* MemoryManager_AllocPage(struct MemoryManager *mm, size_t size) {
    if (!mm || !mm->internal) return NULL;

    PageCacheInternal *internal = mm->internal;
    pthread_mutex_lock(&internal->lock);

    // First, try to allocate from PageCache
    Page* cached_page = PageCache_Alloc(&internal->page_cache);
    if (cached_page) {
        pthread_mutex_unlock(&internal->lock);
        return cached_page->data;
    }

    // If PageCache allocation fails, use mmap
    void *new_page = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new_page == MAP_FAILED) {
        pthread_mutex_unlock(&internal->lock);
        return NULL;
    }
    size_t new_num_pages = internal->num_pages + 1;
    void **new_pages = realloc(internal->pages, new_num_pages * sizeof(void*));
    size_t *new_page_sizes = realloc(internal->page_sizes, new_num_pages * sizeof(size_t));

    if (!new_pages || !new_page_sizes) {
        munmap(new_page, size);
        pthread_mutex_unlock(&internal->lock);
        return NULL;
    }

    internal->pages = new_pages;
    internal->page_sizes = new_page_sizes;
    internal->pages[internal->num_pages] = new_page;
    internal->page_sizes[internal->num_pages] = size;
    internal->num_pages = new_num_pages;

    pthread_mutex_unlock(&internal->lock);
    return new_page;
}

void MemoryManager_FreePage(struct MemoryManager *mm, void *ptr) {
    if (!mm || !mm->internal || !ptr) return;

    PageCacheInternal *internal = mm->internal;
    pthread_mutex_lock(&internal->lock);

    for (size_t i = 0; i < internal->num_pages; ++i) {
        if (internal->pages[i] == ptr) {
            munmap(ptr, internal->page_sizes[i]);

            // Remove the page from the arrays
            memmove(&internal->pages[i], &internal->pages[i+1], (internal->num_pages - i - 1) * sizeof(void*));
            memmove(&internal->page_sizes[i], &internal->page_sizes[i+1], (internal->num_pages - i - 1) * sizeof(size_t));

            internal->num_pages--;

            // Reallocate arrays if necessary
            if (internal->num_pages > 0) {
                internal->pages = realloc(internal->pages, internal->num_pages * sizeof(void*));
                internal->page_sizes = realloc(internal->page_sizes, internal->num_pages * sizeof(size_t));
            } else {
                free(internal->pages);
                free(internal->page_sizes);
                internal->pages = NULL;
                internal->page_sizes = NULL;
            }

            break;
        }
    }

    pthread_mutex_unlock(&internal->lock);
}

void MemoryManager_Destroy(struct MemoryManager *mm) {
    if (!mm || !mm->internal) return;

    PageCacheInternal *internal = mm->internal;
    pthread_mutex_lock(&internal->lock);

    for (size_t i = 0; i < internal->num_pages; ++i) {
        munmap(internal->pages[i], internal->page_sizes[i]);
    }

    free(internal->pages);
    free(internal->page_sizes);

    pthread_mutex_unlock(&internal->lock);
    pthread_mutex_destroy(&internal->lock);

    free(internal);
    mm->internal = NULL;
}