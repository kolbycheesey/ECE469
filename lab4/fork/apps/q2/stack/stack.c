#include "usertraps.h"
#include "misc.h"

void func(int a) {
  int b;
  if (a == 0) return;
  func(a - 1);;
}

void main(int argc, char *argv[]) {
  sem_t s_procs_completed;
  int i;
  if (argc != 3) {
    Printf("Wrong number of arguments\n");
  }
  for (i = 0; i < argc; i++) {
    Printf("%s\n", argv[i]);
  }
  s_procs_completed = dstrtol(argv[2], 10, NULL);
  Printf("test\n");
  func(10000);
  if (sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Failed to signal semaphore!\n");
    Exit();
  }
}
