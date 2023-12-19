#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "shared.h"

void main (int argc, char *argv[])
{
  int numprocs = 0;               // Used to store number of processes to create
  int i;
  uint32 h_mem;                   // Used to hold handle to shared memory page
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  lock_t lock;                    //lock
  //condition variables
  cond_t not_empty;
  cond_t not_full;
  Data *data = NULL;              // Shared data initialized to NULL
  char h_mem_str[10];             // Used as command-line argument to pass mem_handle to new processes
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  char lock_str[10]; 
  char not_full_str[10]; 
  char not_empty_str[10]; 

  if (argc != 2) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  numprocs = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("Creating %d processes\n", numprocs);

  if ((h_mem = shmget()) == 0) {
    Printf("ERROR: could not allocate shared memory page in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  if ((data = (Data *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
    Exit();
  }

  // Put some values in the shared memory, to be read by other processes
  data->numprocs = numprocs;
  initQueue(&data->q);

  if ((s_procs_completed = sem_create(-(numprocs * 2 - 1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((lock = lock_create()) == SYNC_FAIL) {
    Printf("Failed to create lock");
    Exit();
  }
  if ((not_full = cond_create(lock)) == SYNC_FAIL) {
    Printf("Cond variable creation failed\n");
    Exit();
  }
  if ((not_empty = cond_create(lock)) == SYNC_FAIL) {
    Printf("Cond variable creation failed\n");
    Exit();
  }
//  Printf("Created condition variables\n");
  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(h_mem, h_mem_str);
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(lock, lock_str);
  ditoa(not_empty, not_empty_str);
  ditoa(not_full, not_full_str);

  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  for(i = 0; i<numprocs; i++) {
    process_create("consumer.dlx.obj", h_mem_str, s_procs_completed_str, lock_str, not_empty_str, not_full_str, NULL);
    process_create("producer.dlx.obj", h_mem_str, s_procs_completed_str, lock_str, not_empty_str, not_full_str, NULL);
    Printf("Process %d created\n", i);
  }

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");
}
