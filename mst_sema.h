#ifndef MST_SEMA_H
#define MST_SEMA_H

struct mst_sema_s;
typedef struct mst_sema_s *mst_sema_t;

mst_sema_t mst_sema_create(int initial);
void mst_sema_signal(mst_sema_t s);
void mst_sema_wait(mst_sema_t s);

// resets sema to its initial value
// all waiting threads will continue to run
void mst_sema_reset(mst_sema_t s);

#endif
