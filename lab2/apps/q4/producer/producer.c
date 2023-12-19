#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "shared.h"

void main (int argc, char *argv[]) {
  Data *data;
  uint32 h_mem;
  sem_t s_procs_completed;
  lock_t lock;
  cond_t not_full;
  cond_t not_empty;
  char buf[HELLO_WORLD_LEN];
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
  strcpy(buf, "Hello World");
  for (i = 0; i < HELLO_WORLD_LEN; i++) {
    lock_acquire(lock);
    //while (data->q.size == HELLO_WORLD_LEN);
    if (data->q.size == 0) {
      cond_signal(not_empty);
      enqueue(&data->q, buf[i]);
    }
    if (data->q.size < HELLO_WORLD_LEN) {
      enqueue(&data->q, buf[i]);
      Printf("Producer %d inserted: %c\n", getpid(), buf[i]);
    }
    else {
      cond_wait(not_full);
      lock_acquire(lock);
      i--;
    }
    lock_release(lock);
  }
  if (sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Exit();
  }
}
