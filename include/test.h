//
// Created by a2006 on 24-7-25.
//


// test.h
//
// Created by a2006 on 24-7-25.
//

// test.h
#ifndef TEST_H
#define TEST_H

#include <stddef.h>

// 可配置的测试参数
#define NUM_THREADS 3
#define ALLOCATIONS_PER_THREAD 4000
#define TOTAL_ALLOCATIONS (NUM_THREADS * ALLOCATIONS_PER_THREAD)

// 新增：可配置的内存块大小
#define BLOCK_SIZE 16

// 测试函数声明
void run_allocation_test(int num_threads, int allocations_per_thread);

#endif // TEST_H