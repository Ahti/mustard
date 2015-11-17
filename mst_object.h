#ifndef MST_OBJECT_H
#define MST_OBJECT_H

#include <stddef.h>
#include <pthread.h>
#include "mst_sema.h"

typedef union mst_object_u mst_object_t;

typedef void (* mst_object_destructor)(mst_object_t object);

struct mst_object_s {
    char *name;
    int retain_count;
    mst_object_destructor destructor;
    pthread_mutex_t mutex;
};
union mst_object_u {
    struct mst_object_s *obj;
    mst_sema_t sema;
    void *any;
} __attribute__((transparent_union));


void mst_retain(void *obj);
void mst_release(void *obj);

#define mst_object_create(type) \
((type##_t) _mst_object_create(sizeof(struct type##_s), (mst_object_destructor) type##_destruct, #type))
void * _mst_object_create(size_t size, mst_object_destructor destructor, char *type_name);

#endif
