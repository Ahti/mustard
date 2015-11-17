#include "mst_group.h"
#include "mst_object.h"
#include "mst_sema.h"
#include <pthread.h>
#include <stdlib.h>
#include "queue.h"

struct mst_group_s {
    struct mst_object_s _obj;
    int count;
    pthread_mutex_t m;

    mst_queue_t callback_q;
    mst_fun callback_f;
    void *callback_ctx;
};

void mst_group_destruct(mst_group_t g) {
    pthread_mutex_destroy(&g->m);
    if (g->callback_q != NULL) {
        mst_release(g->callback_q);
    }
}

mst_group_t mst_group_create() {
    mst_group_t g = mst_object_create(mst_group);
    g->count = 0;
    pthread_mutex_init(&g->m, NULL);
    return g;
}

void _mst_group_run_callback(mst_group_t g) {
    if (g->callback_f != NULL) {
        if (g->callback_q != NULL) {
            mst_async(g->callback_q, g->callback_f, g->callback_ctx);
            mst_release(g->callback_q);
            g->callback_q = NULL;
        }
        else {
            g->callback_f(g->callback_ctx);
        }
        g->callback_f = NULL;
    }
}

void mst_group_enter(mst_group_t g) {
    pthread_mutex_lock(&g->m);
    {
        g->count++;
    }
    pthread_mutex_unlock(&g->m);
}
void mst_group_leave(mst_group_t g) {
    pthread_mutex_lock(&g->m);
    {
        g->count--;
    }

    if (g->count <= 0) {
        _mst_group_run_callback(g);
    }
    pthread_mutex_unlock(&g->m);
}

void mst_group_set_callback(mst_group_t g, mst_queue_t q, mst_fun f, void *ctx) {
    pthread_mutex_lock(&g->m);
    {
        g->callback_q = q;
        if (q != NULL) mst_retain(q);
        g->callback_f = f;
        g->callback_ctx = ctx;
    }
    pthread_mutex_unlock(&g->m);
}

void mst_group_wait(mst_group_t g) {
    mst_sema_t s = mst_sema_create(0);
    mst_group_set_callback(g, NULL, (mst_fun)mst_sema_signal, s);
    mst_sema_wait(s);
    mst_release(s);
}

struct mst_group_job {
    mst_group_t g;
    mst_fun f;
    void *ctx;
};

void _mst_group_run_job(struct mst_group_job *j) {
    j->f(j->ctx);
    mst_group_leave(j->g);
    free(j);
}

void mst_group_async(mst_queue_t q, mst_group_t g, mst_fun f, void *ctx) {
    mst_group_enter(g);
    struct mst_group_job *j = calloc(1, sizeof(struct mst_group_job));
    j->g = g;
    j->f = f;
    j->ctx = ctx;
    mst_async(q, (mst_fun)_mst_group_run_job, j);
}
