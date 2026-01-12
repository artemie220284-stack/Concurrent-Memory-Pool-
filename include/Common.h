//
// Created by a2006 on 24-7-20.
//

// Common.h
#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

// 内存对齐宏
#define ALIGN(size, alignment) (((size) + (alignment) - 1) & ~((alignment) - 1))

// 最小和最大宏
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// 页大小（假设为4KB，你可以根据实际情况调整）
#define PAGE_SIZE 4096

// 内存大小单位
#define KB (1024ULL)
#define MB (1024ULL * KB)
#define GB (1024ULL * MB)

// 错误码
typedef enum {
    ERROR_NONE = 0,
    ERROR_OUT_OF_MEMORY,
    ERROR_INVALID_ARGUMENT,
    ERROR_NOT_INITIALIZED
} ErrorCode;

// 线程安全的原子操作（如果你的编译器支持C11原子操作）
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
#include <stdatomic.h>
#define ATOMIC(type) _Atomic type
#define ATOMIC_LOAD(ptr) atomic_load(ptr)
#define ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
#define ATOMIC_EXCHANGE(ptr, val) atomic_exchange(ptr, val)
#define ATOMIC_COMPARE_EXCHANGE_WEAK(ptr, expected, desired) \
    atomic_compare_exchange_weak(ptr, expected, desired)
#else
// 如果不支持C11原子操作，你可能需要使用其他同步原语，比如互斥锁
#include <pthread.h>
#define ATOMIC(type) type
#define ATOMIC_LOAD(ptr) (*(ptr))
#define ATOMIC_STORE(ptr, val) (*(ptr) = (val))
// 注意：这些宏在非原子环境下不是线程安全的
#endif

#endif // COMMON_H