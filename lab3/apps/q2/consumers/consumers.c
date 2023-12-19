#include "usertraps.h"
#include "misc.h"

void main(int argc, char **argv) {
  mbox_t m1;
  mbox_t m2;
  sem_t processes_waiting;
  char buf[20];
  char buf2[4];

  Printf("Consumer PID: %d\n", getpid());
  if (argc != 5) {
    Printf("Wrong number of arguments!\n");
    Exit();
  }
  
  m1 = dstrtol(argv[1], NULL, 10);
  m2 = dstrtol(argv[2], NULL, 10);

  
  if (mbox_open(m1) == MBOX_FAIL) {
    Printf("mbox opening failed!\n");
    Exit();
  }
  if (mbox_open(m2) == MBOX_FAIL) {
    Printf("mbox opening failed!\n");
    Exit();
  }

  processes_waiting = dstrtol(argv[3], NULL, 10);
  if (!dstrncmp(argv[4], "1", 1)) {
  //  Printf("Receiving\n");
    Printf("Reaction 1\n");
    mbox_recv(m1, 20, (void *)buf);
   // Printf("Received\n");
    dstrcat(buf, " -> S + S");
    dstrcpy(buf2, "S");
    mbox_send(m2, dstrlen("S"), (void *)buf2);
    mbox_send(m2, dstrlen("S"), (void *)buf2);
    Printf("%s\n", buf);
  }
  else if (!dstrncmp(argv[4], "2", 1)) {
    Printf("Reaction 2\n");
    mbox_recv(m1, dstrlen(buf), (void *)buf);
    mbox_recv(m1, dstrlen(buf), (void *)buf);
    mbox_recv(m1, dstrlen(buf), (void *)buf);
    mbox_recv(m1, dstrlen(buf), (void *)buf);
    mbox_send(m2, dstrlen("2C2"), (void *)buf2);
    Printf("4CO -> 2O2 + 2C2\n", buf);
  }
  else if (!dstrncmp(argv[5], "3", 1 )) {
    Printf("Reaction 3\n");
    mbox_recv(m1, dstrlen(buf), (void *)buf);
    mbox_recv(m2, dstrlen(buf), (void *)buf2);
    dstrcat(buf, " + ");
    dstrcat(buf, buf2);
    dstrcat(buf, " -> SO4");
    Printf("%s\n", buf);
  }
  if (sem_signal(processes_waiting) == SYNC_FAIL) {
    Printf("Failed to signal semaphore\n");
    Exit();
  }
//  Printf("Consumer Exit\n");
}
