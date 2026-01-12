#include "test.h"
#include "UnLocked.h"
#include "ThreadPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "MemoryManager.h"
#include "ConcurrentAlloc.h"

Allocator allocator;
ThreadPool* pool;
#define BATCH_SIZE 1000  // 每次批量分配/释放的数量

typedef struct {
    int allocations;
    void** ptrs;
    int thread_id;
} ThreadArg;

void allocation_task(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;
    for (int i = 0; i < thread_arg->allocations; i += BATCH_SIZE) {
        int batch = (thread_arg->allocations - i < BATCH_SIZE) ? (thread_arg->allocations - i) : BATCH_SIZE;
        for (int j = 0; j < batch; ++j) {
            size_t size = (rand() % 4 + 1) * 16;  // 随机选择 16, 32, 64, 或 128
            void* ptr = alloc(&allocator, size);
            if (ptr == NULL) {
                fprintf(stderr, "Thread %d: 内存分配失败 at iteration %d\n", thread_arg->thread_id, i + j);
                continue;
            }
            thread_arg->ptrs[i + j] = ptr;
        }
    }
}

void deallocation_task(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;
    for (int i = 0; i < thread_arg->allocations; i += BATCH_SIZE) {
        int batch = (thread_arg->allocations - i < BATCH_SIZE) ? (thread_arg->allocations - i) : BATCH_SIZE;
        for (int j = 0; j < batch; ++j) {
            if (thread_arg->ptrs[i + j] != NULL) {
                custom_free(&allocator, thread_arg->ptrs[i + j]);
            }
        }
    }
    ConcurrentFree(thread_arg->ptrs);
    ConcurrentFree(thread_arg);
}

void run_allocation_test(int num_threads, int allocations_per_thread) {
    struct timespec start, end;
    double cpu_time_used;

    printf("测试参数：线程数 = %d, 每线程分配次数 = %d\n", num_threads, allocations_per_thread);

    clock_gettime(CLOCK_MONOTONIC, &start);

    ThreadArg** args = ConcurrentAlloc(sizeof(ThreadArg*) * num_threads);
    if (!args) {
        fprintf(stderr, "Failed to allocate memory for args array\n");
        return;
    }

    for (int i = 0; i < num_threads; ++i) {
        args[i] = ConcurrentAlloc(sizeof(ThreadArg));
        if (!args[i]) {
            fprintf(stderr, "Failed to allocate memory for thread argument %d\n", i);
            for (int j = 0; j < i; ++j) {
                ConcurrentFree(args[j]->ptrs);
                ConcurrentFree(args[j]);
            }
            ConcurrentFree(args);
            return;
        }
        args[i]->allocations = allocations_per_thread;
        args[i]->ptrs = ConcurrentAlloc(allocations_per_thread * sizeof(void*));
        args[i]->thread_id = i;
        if (!args[i]->ptrs) {
            fprintf(stderr, "Failed to allocate memory for thread %d pointers\n", i);
            for (int j = 0; j <= i; ++j) {
                ConcurrentFree(args[j]);
            }
            ConcurrentFree(args);
            return;
        }
        thread_pool_submit(pool, allocation_task, args[i]);
    }

    // 等待所有任务完成
    while (atomic_load(&pool->task_count) > 0) {
        sched_yield();  // 让出CPU，减少忙等待
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    cpu_time_used = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    long total_allocations = (long)num_threads * allocations_per_thread;
    double allocs_per_second = total_allocations / cpu_time_used;

    printf("总共分配了 %ld 次内存\n", total_allocations);
    printf("总耗时: %f 秒\n", cpu_time_used);
    printf("分配速度: %.2f 次/秒\n", allocs_per_second);

    // 清理内存
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; ++i) {
        thread_pool_submit(pool, deallocation_task, args[i]);
    }

    // 等待所有任务完成
    while (atomic_load(&pool->task_count) > 0) {
        sched_yield();  // 让出CPU，减少忙等待
    }

    ConcurrentFree(args);

    clock_gettime(CLOCK_MONOTONIC, &end);
    cpu_time_used = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double frees_per_second = total_allocations / cpu_time_used;

    printf("释放速度: %.2f 次/秒\n", frees_per_second);
}

int main() {
    srand(time(NULL));

    if (!InitializeMemoryManager()) {
        fprintf(stderr, "Failed to initialize memory manager\n");
        return EXIT_FAILURE;
    }

    init_allocator(&allocator);
    pool = thread_pool_create(NUM_THREADS, NUM_THREADS * 2);
    if (!pool) {
        fprintf(stderr, "Failed to create thread pool\n");
        DestroyMemoryManager();
        return EXIT_FAILURE;
    }

    // 运行测试
    run_allocation_test(NUM_THREADS, ALLOCATIONS_PER_THREAD);

    // 压力测试
    printf("\n开始压力测试\n");
    run_allocation_test(NUM_THREADS * 2, ALLOCATIONS_PER_THREAD * 2);

    thread_pool_destroy(pool);
    DestroyMemoryManager();

    return EXIT_SUCCESS;
}