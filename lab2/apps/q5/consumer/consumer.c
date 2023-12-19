#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[]) {
  uint32 h_mem;
  sem_t num_procs;
  sem_t mol1;
  sem_t mol2;
  sem_t mol3;
  int i;
  int num_reactions;
  if (argc != 7) Exit();
  mol1 = dstrtol(argv[2], NULL, 10);
  mol2 = dstrtol(argv[3], NULL, 10);
  mol3 = dstrtol(argv[4], NULL, 10);
  num_procs = dstrtol(argv[5], NULL, 10);
  num_reactions = dstrtol(argv[6], NULL, 10);
  if (!dstrncmp(argv[1], "1", 1)) {
    for (i = 0; i < num_reactions; i++) {
      sem_wait(mol1);
      sem_wait(mol1);
      sem_signal(mol2);
      sem_signal(mol2);
      sem_signal(mol3);
      Printf("2 H2O -> 2 H2 + O2 reacted, PID %d\n", getpid());
    }
  }
  else if (!dstrncmp(argv[1], "2", 1)){
    for (i = 0; i < num_reactions; i++) {
      sem_wait(mol1);
      sem_signal(mol2);
      sem_signal(mol3);
      Printf("SO4 -> SO2 + O2 reacted, PID %d\n", getpid());
    }
  }
  else {
    for (i = 0; i < num_reactions; i++) {
      sem_wait(mol1);
      sem_wait(mol2);
      sem_wait(mol3);
      Printf("(%d) H2 + O2 + SO2 -> H2SO4, PID %d\n", i + 1, getpid());
    }
  }
  sem_signal(num_procs);
}
