#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "shared.h"
void main (int argc, char *argv[]) {
  Data *data;
  uint32 h_mem;
  sem_t s_procs_completed;
  lock_t lock;
  cond_t not_empty;
  cond_t not_full;
  int full = 0;
  int i;
  if (argc != 6) {
    Exit();
  }
  h_mem = dstrtol(argv[1], NULL, 10);
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  lock = dstrtol(argv[3], NULL, 10);
  not_empty = dstrtol(argv[4], NULL, 10);
  not_full = dstrtol(argv[5], NULL, 10);
  if (!(data = (Data *)shmat(h_mem))) {
    Exit();
  }
  for (i = 0; i < HELLO_WORLD_LEN; i++) {
    lock_acquire(lock);
    if (!data->q.size) {
      cond_wait(not_empty);
      lock_acquire(lock);
    }
    else if (data->q.size  < HELLO_WORLD_LEN) {
      Printf("Consumer %d removed: %c\n", getpid(), dequeue(&data->q));
    }
    else {
      cond_signal(not_full);
      i--;
    }
    lock_release(lock);
  }
  if (sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Exit();
  }
}
