#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#define min(a, b) a < b ? a : b
void main (int argc, char *argv[])
{
  //int numprocs = 0;               // Used to store number of processes to create
  //  int i;
  //molecule counts
  int num_h2o;
  int num_so4;
  int num_h2;
  int num_o2;
  int num_so2;
  int num_h2so4;
  int rounded_num_h2o;
  int numr1;
  int numr2;
  int numr3;
  int processes;

  //semaphore declarations
  sem_t h2o_sem;
  sem_t so4_sem;
  sem_t h2_sem;
  sem_t o2_sem;
  sem_t so2_sem;
  sem_t num_procs;
  
  //strings 
  char h2o_sem_str[10];
  char so4_sem_str[10];
  char h2_sem_str[10];
  char o2_sem_str[10];
  char so2_sem_str[10];
  char num_procs_str[10];
  char num_h2o_str[10];
  char num_so4_str[10];
  char molecule[10];
  char rxn[10];
  char numr1_str[10];
  char numr2_str[10];
  char numr3_str[10];

  if (argc != 3) {
    Printf("Wrong number of args!\n");
    Exit();
  }
  num_h2o = dstrtol(argv[1], NULL, 10);
  num_so4 = dstrtol(argv[2], NULL, 10);
  Printf("Creating %d H2Os and %d SO4s.\n", num_h2o, num_so4);
  rounded_num_h2o = (num_h2o / 2) * 2;
  num_h2 = rounded_num_h2o;
  num_o2 = num_h2 / 2;
  num_so2 = num_so4;
  num_o2 += num_so4;
  num_h2so4 = min(min(num_h2, num_so2), num_o2);

  numr1 = num_h2 / 2;
  numr2 = num_so4;
  numr3 = num_h2so4;

  processes = 5;
  if ((h2o_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Failed to create semaphore!\n");
    Exit();
  }
  if ((so4_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Failed to create semaphore!\n");
    Exit();
  }
  if ((h2_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Failed to create semaphore!\n");
    Exit();
  }
  if ((o2_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Failed to create semaphore!\n");
    Exit();
  }
  if ((so2_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Failed to create semaphore!\n");
    Exit();
  }

  if ((num_procs = sem_create(-(processes - 1))) == SYNC_FAIL) {
    Printf("Failed to create semaphore!\n");
    Exit();
  }
  ditoa(h2o_sem, h2o_sem_str);
  ditoa(so4_sem, so4_sem_str);
  ditoa(h2_sem, h2_sem_str);
  ditoa(o2_sem, o2_sem_str);
  ditoa(so2_sem, so2_sem_str);
  ditoa(num_h2o, num_h2o_str);
  ditoa(num_so4, num_so4_str);
  ditoa(num_procs, num_procs_str);
  ditoa(numr1, numr1_str);
  ditoa(numr2, numr2_str);
  ditoa(numr3, numr3_str);
  dstrcpy(molecule, "H2O\0");
  process_create("producer.dlx.obj", h2o_sem_str, num_h2o_str, molecule, num_procs_str, NULL);
  dstrcpy(rxn, "1\0");
  process_create("consumer.dlx.obj", rxn, h2o_sem_str, h2_sem_str, o2_sem_str, num_procs_str, numr1_str, NULL);
  dstrcpy(molecule, "SO4\0");
  process_create("producer.dlx.obj", so4_sem_str, num_so4_str, molecule, num_procs_str, NULL);
  dstrcpy(rxn, "2\0");
  process_create("consumer.dlx.obj", rxn, so4_sem_str, so2_sem_str, o2_sem_str, num_procs_str, numr2_str, NULL);
  dstrcpy(rxn, "3\0");
  process_create("consumer.dlx.obj", rxn, h2_sem_str, o2_sem_str, so2_sem_str, num_procs_str, numr3_str, NULL);
  sem_wait(num_procs);
  Printf("%d H2O's left over. %d H2's left over. %d O2's left over. %d SO2's left over." ,num_h2o - rounded_num_h2o, num_h2 - num_h2so4, num_o2 - num_h2so4 , num_so2 - num_h2so4);
  Printf(" %d H2SO4's created\n", num_h2so4);
  Printf("All other processes completed, exiting main process.\n");
}
