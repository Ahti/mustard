#include "mst_object.h"
#include "mst_sema.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef __APPLE__

#include <dispatch/dispatch.h>
struct mst_sema_s {
    struct mst_object_s _obj;
    dispatch_semaphore_t s;
    int initial;
    int val;
};

#else

#include <semaphore.h>
struct mst_sema_s {
    struct mst_object_s _obj;
    sem_t s;
    int initial;
    int val;
};

#endif

void mst_sema_destruct(mst_sema_t s) {
    assert(s->val == s->initial);
    #ifdef __APPLE__
    dispatch_release(s->s);
    #else
    sem_destroy(&s->s);
    #endif
}

mst_sema_t mst_sema_create(int initial) {
    mst_sema_t s = mst_object_create(mst_sema);
    s->val = initial;
    s->initial = initial;
    #ifdef __APPLE__
    s->s = dispatch_semaphore_create(initial);
    #else
    sem_init(&s->s, 0, initial);
    #endif
    return s;
}

void mst_sema_wait(mst_sema_t s) {
    s->val--;
    #ifdef __APPLE__
    dispatch_semaphore_wait(s->s, DISPATCH_TIME_FOREVER);
    #else
    sem_wait(&s->s);
    #endif
}

void mst_sema_signal(mst_sema_t s) {
    s->val++;
    #ifdef __APPLE__
    dispatch_semaphore_signal(s->s);
    #else
    sem_post(&s->s);
    #endif
}

void mst_sema_reset(mst_sema_t s) {
    while (s->val < s->initial) {
        mst_sema_signal(s);
    }
    while (s->val > s->initial) {
        mst_sema_wait(s);
    }
}
