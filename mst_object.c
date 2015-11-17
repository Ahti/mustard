#include "mst_object.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void mst_retain(void *obj) {
    struct  mst_object_s *o = obj;
    pthread_mutex_lock(&o->mutex);
    o->retain_count++;
    pthread_mutex_unlock(&o->mutex);
}

void mst_release(void *obj) {
    struct mst_object_s *o = obj;
    bool did_destroy = false;
    pthread_mutex_lock(&o->mutex);

    o->retain_count--;
    if (o->retain_count <= 0) {
        o->destructor((mst_object_t)o);
        did_destroy = true;
    }

    pthread_mutex_unlock(&o->mutex);

    if (did_destroy) {
        pthread_mutex_destroy(&o->mutex);
        free(o);
    }
}

void * _mst_object_create(size_t size, mst_object_destructor destructor, char *type_name) {
    struct mst_object_s *o = calloc(size, 1);
    o->name = type_name;
    o->destructor = destructor;
    o->retain_count = 1;
    pthread_mutex_init(&o->mutex, NULL);
    return o;
}
