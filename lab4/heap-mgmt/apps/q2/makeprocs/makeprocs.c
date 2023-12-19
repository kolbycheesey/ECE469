#include "usertraps.h"
#include "misc.h"

void main(int argc, char *argv[]) {
  int num_procs;
  sem_t s_procs_completed;
  char s_procs_completed_str[10];
  int i;

  num_procs = 35;
  if ((s_procs_completed = sem_create(-(num_procs - 1))) == SYNC_FAIL) {
    Printf("Failed to create semaphore!\n");
    Exit();
  }
  ditoa(s_procs_completed, s_procs_completed_str);

  //Hello World
  process_create("spawn_me.dlx.obj", "1", s_procs_completed_str, NULL);


  //Access outside allocated pages (should get segfault)
  process_create("spawn_me.dlx.obj", "3", s_procs_completed_str, NULL);

  //allocate user stack to more than one page (should get pagefault)
  process_create("spawn_me.dlx.obj", "4", s_procs_completed_str, NULL);

  //hello world program 100 times
  process_create("spawn_me.dlx.obj", "5", s_procs_completed_str, NULL);

  //spawn 30 processes
  for (i = 0; i < 30; i++) {
    process_create("spawn_me.dlx.obj", "6", s_procs_completed_str, NULL);
  }

  //Access beyond virtual memory
  process_create("spawn_me.dlx.obj", "2", s_procs_completed_str, NULL);
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore\n");
    Exit();
  }
}
