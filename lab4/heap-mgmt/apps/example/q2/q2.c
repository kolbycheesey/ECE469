#include "usertraps.h"
#include "misc.h"


void main(int argc, char * argv[]){


  sem_t s_procs_completed; //sem used to signal the original process we are done
  

  //Q2.1 print Hello World and exit
  Printf("Hello World: PID(%d)", getpid());
  Exit();

  //Q2.2 access mem beyond max virtual address  
  /*
  *
  *
  */

  //Q2.3 access mem inside virtual address space & outside currently allocated pages 
  /*
  *
  *
  */


  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Exit failure semaphore @ end of q2: PID(%d), s_procs_completed(%d)\n", getpid(), s_procs_completed);
    Exit();
  }

}
