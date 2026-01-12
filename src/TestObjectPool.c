//
// Created by a2006 on 24-7-20.
//
// testobjectpool.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ObjectPool.h"
#include "ConcurrentAlloc.h"

#define NUM_THREADS 10
#define NUM_ALLOCATIONS 1000
#define OBJECT_SIZE 64
#define CHUNK_SIZE 4096
typedef struct {
    int id;
    char data[OBJECT_SIZE - sizeof(int)];
} TestObject;

ObjectPool *pool = NULL;

void* thread_function(void *arg) {
    int thread_id = *(int*)arg;
    TestObject *objects[NUM_ALLOCATIONS];

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        objects[i] = (TestObject*)ObjectPool_Alloc(&pool);
        if (objects[i] == NULL) {
            printf("Thread %d: Failed to allocate object %d\n", thread_id, i);
            continue;
        }
        objects[i]->id = thread_id * NUM_ALLOCATIONS + i;
    }

    // 使用对象...

    // 这里我们不需要显式地释放对象，因为 ObjectPool 管理内存

    printf("Thread %d completed\n", thread_id);
    return NULL;
}
#ifndef RUN_UNIT_TEST


int main() {
    if (!MemoryManager_Init()) {
        fprintf(stderr, "Failed to initialize memory manager\n");
        return EXIT_FAILURE;
    }

    if (!ObjectPool_Init(&pool, sizeof(TestObject), CHUNK_SIZE)) {
        fprintf(stderr, "Failed to initialize object pool\n");
        MemoryManager_Destroy();
        return EXIT_FAILURE;
    }

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            ObjectPool_Destroy(&pool);
            MemoryManager_Destroy();
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    ObjectPool_Destroy(&pool);
    MemoryManager_Destroy();

    printf("Test completed successfully\n");
    return EXIT_SUCCESS;
} // Your object pool test main function

#endif
