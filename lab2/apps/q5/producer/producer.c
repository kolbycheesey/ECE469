#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"


void main (int argc, char *argv[]) {
  int i;
  sem_t molecule_sem;
  sem_t num_procs;
  int num_molecules;
  if (argc != 5) {
    Exit();
  }
  molecule_sem = dstrtol(argv[1], NULL, 10);
  num_molecules = dstrtol(argv[2], NULL, 10);
  num_procs = dstrtol(argv[4], NULL, 10);
  for (i = 0; i < num_molecules; i++) {
    sem_signal(molecule_sem);
    //Printf("count: %d\n", getSemCount(molecule));
    //Printf("%d\n", dstrncmp(which_molecule, "H2O", 3));
    Printf(argv[3]);
    Printf(" injected into Radeon Atmosphere, PID %d\n", getpid());
  }
  sem_signal(num_procs);
}
