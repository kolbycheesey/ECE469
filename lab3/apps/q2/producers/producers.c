#include "usertraps.h"
#include "misc.h"

void main(int argc, char **argv) {

  mbox_t molecule;
  sem_t processes_waiting;
 // char buf[10];

  Printf("Producer PID: %d\n", getpid());
  if (argc != 4) {
    Printf("Wrong number of arguments!\n");
    Exit();
  }

  molecule = dstrtol(argv[1], NULL, 10);
  processes_waiting = dstrtol(argv[2], NULL, 10);

  if (mbox_open(molecule) == MBOX_FAIL) {
    Printf("Failed to open mbox!\n");
    Exit();
  }

  //Printf("%d %s\n", dstrlen(argv[3]), argv[3]);
  if (mbox_send(molecule, dstrlen(argv[3]), (void *)argv[3]) == MBOX_FAIL) {
    Printf("Failed to send message\n");
  }
  if (sem_signal(processes_waiting) == SYNC_FAIL) {
    Printf("Failed to signal semaphore\n");
    Exit();
  }
  //Printf("Producer Exit\n");
}
