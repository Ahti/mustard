#ifndef MST_GROUP_H
#define MST_GROUP_H

#include "mst_queue.h"

struct mst_group_s;
typedef struct mst_group_s *mst_group_t;

mst_group_t mst_group_create();
void mst_group_enter(mst_group_t g);
void mst_group_leave(mst_group_t g);

void mst_group_set_callback(mst_group_t g, mst_queue_t q, mst_fun f, void *ctx);
void mst_group_wait(mst_group_t g);

void mst_group_async(mst_queue_t q, mst_group_t g, mst_fun f, void *ctx);
#endif
