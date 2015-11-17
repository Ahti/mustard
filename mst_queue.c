#include "mst_object.h"
#include "mst_queue.h"
#include "mst_sema.h"
#include "queue.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

void mst_queue_spawn_workers(mst_queue_t q);
void *mst_worker_main(void *q);

struct mst_queue_s {
    struct mst_object_s _obj;
    queue_t op_queue;
    int num_workers;
    pthread_t *workers;
};

struct mst_job_s {
    struct mst_object_s _obj;
    mst_fun f;
    void *ctx;
};
typedef struct mst_job_s *mst_job_t;
void mst_job_destruct(mst_job_t j) {
    //
}

mst_job_t mst_job_create(mst_fun f, void *ctx) {
    mst_job_t j = mst_object_create(mst_job);
    j->f = f;
    j->ctx = ctx;
    return j;
}

void mst_queue_destruct(mst_queue_t q) {
    queue_destroy(q->op_queue);
    for (int i = 0; i < q->num_workers; i++) {
        pthread_cancel(q->workers[i]);
    }
    free(q->workers);
}

mst_queue_t mst_queue_create(int num_threads) {
    mst_queue_t q = mst_object_create(mst_queue);
    q->op_queue = queue_create();
    q->num_workers = num_threads;
    mst_queue_spawn_workers(q);
    return q;
}

void mst_queue_spawn_workers(mst_queue_t q) {
    pthread_t *threads = calloc(q->num_workers, sizeof(pthread_t));
    for(int i = 0; i < q->num_workers; i++) {
        pthread_create(&threads[i], NULL, mst_worker_main, q->op_queue);
        pthread_detach(threads[i]);
    }
    q->workers = threads;
}

void mst_async(mst_queue_t m, mst_fun f, void *ctx) {
    queue_enqueue(m->op_queue, mst_job_create(f, ctx));
}

void *mst_worker_main(void *_q) {
    queue_t q = _q;

    mst_job_t cur;
    while ((cur = queue_dequeue_blocking(q))) {
        cur->f(cur->ctx);
        mst_release(cur);
    }

    return NULL;
}

struct mst_sync_job_s {
    mst_fun f;
    void *ctx;
    mst_sema_t sema;
};
typedef struct mst_sync_job_s mst_sync_job_t;

void mst_perform_sync_job(void *_j) {
    mst_sync_job_t *j = _j;
    j->f(j->ctx);
    mst_sema_signal(j->sema);
}

void mst_sync(mst_queue_t m, mst_fun f, void *ctx) {
    mst_sema_t s = mst_sema_create(0);
    mst_sync_job_t *j = calloc(sizeof(mst_sync_job_t), 1);
    j->f = f;
    j->ctx = ctx;
    j->sema = s;
    mst_async(m, mst_perform_sync_job, j);
    mst_sema_wait(s);
    mst_release(s);
    free(j);
}
