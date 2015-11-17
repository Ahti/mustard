#ifndef MST_QUEUE_H
#define MST_QUEUE_H

typedef void (* mst_fun)(void *);

struct mst_queue_s;
typedef struct mst_queue_s *mst_queue_t;

mst_queue_t mst_queue_create(int num_threads);
void mst_async(mst_queue_t m, mst_fun f, void *ctx);

void mst_sync(mst_queue_t m, mst_fun f, void *ctx);

#endif
