#include "usertraps.h"
#include "misc.h"

#define min(a, b) a < b ? a : b
void main(int argc, char **argv) {
  int num_s2;
  int num_co;
  int num_2o2;
  int num_s;
  int leftover_co;

  int num_r1;
  int num_r2;
  int num_r3;
  int num_procs;

  mbox_t mbox_s2;
  mbox_t mbox_co;
  mbox_t mbox_2o2;
  mbox_t mbox_s;

  sem_t processes_waiting;

  char mbox_s2_str[10];
  char mbox_co_str[10];
  char mbox_2o2_str[10];
  char mbox_s_str[10];
  char processes_waiting_str[10];

  int i;
  if (argc != 3) {
    Printf("Supplied Wrong Number of Arguments!\n");
    Exit();
  }

  num_s2 = dstrtol(argv[1], NULL, 10);
  num_co = dstrtol(argv[2], NULL, 10);
  num_s = num_s2 * 2;
  num_2o2 = num_co / 4;

  leftover_co = num_co - 4 * (num_co / 4);
  num_r1 = num_s2;
  num_r2 = num_co / 4;
  num_r3 = min(num_s, num_2o2);

  num_procs = num_s2 + num_co - leftover_co + num_r1 + num_r2 + num_r3;
  Printf("Number of processes to generate: %d\n", num_procs);
  if ((processes_waiting = sem_create(-(num_procs - 1))) == SYNC_FAIL) {
    Printf("Semaphore Creation failed!\n");
    Exit();
  }

  //create mailboxes for each molecule
  if ((mbox_s2 = mbox_create()) == MBOX_FAIL) {
    Printf("mbox creation failed!\n");
    Exit();
  }
  if ((mbox_co = mbox_create()) == MBOX_FAIL) {
    Printf("mbox creation failed!\n");
    Exit();
  }
  if ((mbox_2o2 = mbox_create()) == MBOX_FAIL) {
    Printf("mbox creation failed!\n");
    Exit();
  }
  if ((mbox_s = mbox_create()) == MBOX_FAIL) {
    Printf("mbox creation failed!\n");
    Exit();
  }

  //open mailboxes
  if (mbox_open(mbox_s2) == MBOX_FAIL) {
    Printf("mbox opening failed!\n");
  }
  if (mbox_open(mbox_co) == MBOX_FAIL) {
    Printf("mbox opening failed!\n");
  }
  if (mbox_open(mbox_2o2) == MBOX_FAIL) {
    Printf("mbox opening failed!\n");
  }
  if (mbox_open(mbox_s) == MBOX_FAIL) {
    Printf("mbox opening failed!\n");
  }
  //convert mbox handles to strings to pass them in as arguments to the child processes
  ditoa(mbox_s2, mbox_s2_str);
  ditoa(mbox_co, mbox_co_str);
  ditoa(mbox_2o2, mbox_2o2_str);
  ditoa(mbox_s, mbox_s_str);
  ditoa(processes_waiting, processes_waiting_str);

  Printf("%s %s %s %s\n", mbox_s2_str, mbox_co_str, mbox_2o2_str, mbox_s_str);
  //generate s2
  Printf("Producing %d s2\n", num_s2);
  for (i = 0; i < num_s2; i++) {
    process_create("producers.dlx.obj", 0, 0, mbox_s2_str, processes_waiting_str, "S2", NULL);
  }
  //generate co
  Printf("Producing %d co\n", num_co - leftover_co);
  for (i = 0; i < num_co - leftover_co; i++) {
    process_create("producers.dlx.obj", 0, 0, mbox_co_str, processes_waiting_str, "CO", NULL);
  }
  //generate 2o2
//  Printf("Producing %d 2o2\n", num_2o2);
 // for (i = 0; i < num_2o2; i++) {
 //   process_create("producers.dlx.obj", 0, 0, mbox_2o2_str, processes_waiting_str, "2O2", NULL);
 // }

  //r1
  Printf("Reaction 1 carrying out %d times\n", num_r1);
  for (i = 0; i < num_r1; i++) {
    process_create("consumers.dlx.obj", 0, 0, mbox_s2_str, mbox_s_str, processes_waiting_str, "1", NULL);
  }
  //r2 
  Printf("Reaction 2 carrying out %d times\n", num_r2);
  for (i = 0; i < num_r2; i++) {
    process_create("consumers.dlx.obj", 0, 0, mbox_co_str, mbox_2o2_str, processes_waiting_str, "2", NULL);
  }
  //r3 
  Printf("Reaction 3 carrying out %d times\n", num_r3);
  for (i = 0; i < num_r3; i++) {
    process_create("consumers.dlx.obj", 0, 0, mbox_co_str, mbox_2o2_str, processes_waiting_str, "3", NULL);
  }
  if (sem_wait(processes_waiting) == SYNC_FAIL) {
    Printf("Error waiting on semaphore!\n");
    Exit();
  }
  Printf("Exited successfully!\n");
}
