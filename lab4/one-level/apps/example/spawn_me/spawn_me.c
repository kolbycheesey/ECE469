#include "usertraps.h"
#include "misc.h"

void func(int a) {
  int b = 3;
  if (a == 0) return;
  func(a-1);
}
void main(int argc, char * argv[]){


  sem_t s_procs_completed; //sem used to signal the original process we are done
  int test;
  int *pointer;
  int i;


  //Q2.1 print Hello World and exit
  if (argc != 3) {
    Printf("Wrong args\n");
    Exit();
  }
  Printf("PID(%d)\n", getpid());
  for (i = 0; i < argc; i++) {
    Printf("%s ", argv[i]);
  }
  Printf("\n");
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  test = dstrtol(argv[1], NULL, 10);
  switch (test) {
    case 1:
      Printf("Hello World: PID(%d)\n", getpid());
      break;
    case 2:
      //first integer beyond last vmem address
      pointer = (int *)(1 << 20);
      *pointer = 5;
      break;
    case 3:
      pointer = (int *)((((1 << 20) - 1) & ~0xfff) - 4); //accesses one page past the page the stack pointer is in
      *pointer = 5;
      break;
    case 4:
      func(500);
      break;
    case 5:
      for (i = 0; i < 100; i++) {
        Printf("Hello World: PID(%d)\n", getpid());
      }
      break;
    case 6: 
      Printf("Message \n");
      for (i = 0; i < 500; i++);
      Printf("Message 2\n");
      break;
    default: break;
  }

  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Exit failure semaphore @ end of q2: PID(%d), s_procs_completed(%d)\n", getpid(), s_procs_completed);
    Exit();
  }

}
