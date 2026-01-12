#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

typedef struct {
    void (*function)(void*);
    void* argument;
} Task;

typedef struct {
    Task* tasks;
    int task_capacity;
    atomic_int task_count;
    int head;
    int tail;
    pthread_t* threads;
    int thread_count;
    atomic_bool stop;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
} ThreadPool;

ThreadPool* thread_pool_create(int thread_count, int task_capacity);
void thread_pool_destroy(ThreadPool* pool);
bool thread_pool_submit(ThreadPool* pool, void (*function)(void*), void* argument);

#endif // THREAD_POOL_H