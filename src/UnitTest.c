#include "ConcurrentAlloc.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "MemoryManager.h"

#ifdef RUN_UNIT_TEST

// Thread function to test ConcurrentAlloc
void* ThreadFunction(void* arg) {
    size_t allocation_size = *(size_t*)arg;
    void* allocated_memory = ConcurrentAlloc(allocation_size);
    if (allocated_memory == NULL) {
        fprintf(stderr, "Memory allocation failed in thread.\n");
    } else {
        printf("Memory allocated successfully in thread: %p\n", allocated_memory);
        ConcurrentFree(allocated_memory);
    }
    return NULL;
}

int main() {
    // Initialize the memory manager
    if (!MemoryManager_Init(&g_memory_manager)) {
        fprintf(stderr, "Memory manager initialization failed.\n");
        return EXIT_FAILURE;
    }

    // Create thread parameters
    size_t allocation_size = 1024; // Allocate 1KB of memory

    // Create two threads
    pthread_t thread1, thread2;
    if (pthread_create(&thread1, NULL, ThreadFunction, &allocation_size) != 0) {
        fprintf(stderr, "Failed to create thread 1.\n");
        MemoryManager_Destroy(&g_memory_manager);
        return EXIT_FAILURE;
    }
    if (pthread_create(&thread2, NULL, ThreadFunction, &allocation_size) != 0) {
        fprintf(stderr, "Failed to create thread 2.\n");
        pthread_join(thread1, NULL); // Wait for thread1 before exiting
        MemoryManager_Destroy(&g_memory_manager);
        return EXIT_FAILURE;
    }

    // Wait for threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Destroy the memory manager
    MemoryManager_Destroy(&g_memory_manager);

    printf("Unit test completed.\n");
    return EXIT_SUCCESS;
}

#endif // RUN_UNIT_TEST