#include "ThreadPool.h"
#include "ConcurrentAlloc.h"
#include <string.h>

static void* worker(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;
    while (true) {
        pthread_mutex_lock(&pool->lock);
        while (atomic_load(&pool->task_count) == 0 && !atomic_load(&pool->stop)) {
            pthread_cond_wait(&pool->not_empty, &pool->lock);
        }
        if (atomic_load(&pool->stop) && atomic_load(&pool->task_count) == 0) {
            pthread_mutex_unlock(&pool->lock);
            return NULL;
        }
        Task task = pool->tasks[pool->head];
        pool->head = (pool->head + 1) % pool->task_capacity;
        atomic_fetch_sub(&pool->task_count, 1);
        pthread_mutex_unlock(&pool->lock);
        task.function(task.argument);
    }
}

ThreadPool* thread_pool_create(int thread_count, int task_capacity) {
    ThreadPool* pool = ConcurrentAlloc(sizeof(ThreadPool));
    if (pool == NULL) {
        return NULL;
    }

    pool->tasks = ConcurrentAlloc(sizeof(Task) * task_capacity);
    if (pool->tasks == NULL) {
        ConcurrentFree(pool);
        return NULL;
    }

    pool->threads = ConcurrentAlloc(sizeof(pthread_t) * thread_count);
    if (pool->threads == NULL) {
        ConcurrentFree(pool->tasks);
        ConcurrentFree(pool);
        return NULL;
    }

    pool->task_capacity = task_capacity;
    atomic_init(&pool->task_count, 0);
    pool->head = 0;
    pool->tail = 0;
    pool->thread_count = thread_count;
    atomic_init(&pool->stop, false);

    if (pthread_mutex_init(&pool->lock, NULL) != 0) {
        ConcurrentFree(pool->threads);
        ConcurrentFree(pool->tasks);
        ConcurrentFree(pool);
        return NULL;
    }

    if (pthread_cond_init(&pool->not_empty, NULL) != 0) {
        pthread_mutex_destroy(&pool->lock);
        ConcurrentFree(pool->threads);
        ConcurrentFree(pool->tasks);
        ConcurrentFree(pool);
        return NULL;
    }

    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker, pool) != 0) {
            atomic_store(&pool->stop, true);
            pthread_cond_broadcast(&pool->not_empty);
            for (int j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            pthread_mutex_destroy(&pool->lock);
            pthread_cond_destroy(&pool->not_empty);
            ConcurrentFree(pool->threads);
            ConcurrentFree(pool->tasks);
            ConcurrentFree(pool);
            return NULL;
        }
    }

    return pool;
}

void thread_pool_destroy(ThreadPool* pool) {
    if (pool == NULL) {
        return;
    }

    atomic_store(&pool->stop, true);
    pthread_cond_broadcast(&pool->not_empty);

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->not_empty);
    ConcurrentFree(pool->tasks);
    ConcurrentFree(pool->threads);
    ConcurrentFree(pool);
}

bool thread_pool_submit(ThreadPool* pool, void (*function)(void*), void* argument) {
    if (pool == NULL || function == NULL) {
        return false;
    }

    pthread_mutex_lock(&pool->lock);

    if (atomic_load(&pool->task_count) == pool->task_capacity) {
        pthread_mutex_unlock(&pool->lock);
        return false;
    }

    pool->tasks[pool->tail] = (Task){function, argument};
    pool->tail = (pool->tail + 1) % pool->task_capacity;
    atomic_fetch_add(&pool->task_count, 1);

    pthread_cond_signal(&pool->not_empty);
    pthread_mutex_unlock(&pool->lock);

    return true;
}