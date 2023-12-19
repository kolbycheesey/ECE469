#include "usertraps.h"
#include "misc.h"

void main(int argc, char *argv[]) {
  int i;
  char *ptr2;
  char *ptr3;
  char *ptr4;
  char *ptr5;
  int *ptr = (int *)malloc(32);
  Printf("%d\n", ptr);
  mfree(ptr);
  ptr2 = (char *)malloc(40);
  ptr3 = (char *)malloc(35);
  ptr4 = (char *)malloc(32);
  ptr5 = (char *)malloc(32);
  mfree(ptr2);
  mfree(ptr3);
  mfree(ptr4);
  mfree(ptr5);
}
