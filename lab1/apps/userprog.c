#include "usertraps.h"

void main (int x)
{
  Printf("Hello World!\n");
  Printf("%d\n", GetPid());
  //Printf("%d", GetPid());
  while(1); // Use CTRL-C to exit the simulator
}
