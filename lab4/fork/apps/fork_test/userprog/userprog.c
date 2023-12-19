#include "usertraps.h"
#include "misc.h"
void printArray(int *array, int size) {
  int i;
  Printf("Array contents\n");
  for (i = 0; i < size; i++) {
    Printf("%d ", array[i]);
  }
  Printf("\n");
}

void main(int argc, char *argv[]) {
  int i;
  int childPid;
  int array[10] = {0};
  if ((childPid = fork()) == 0) {
    for (i = 0; i < 10; i++) {
      array[i] = i;
    }
    printArray(array, 10);
    Printf("Hello from child PID: %d\n", getpid());
  }
  else {
    for (i = 0; i < 10; i++) {
      array[i] = 2 * i;
    }
    printArray(array, 10);
    Printf("Hello from parent PID: %d Child Pid %d\n", getpid(), childPid);
  }
}
