#include "usertraps.h"
#include "misc.h"

void main(int argc, char *argv[]) {
  int i;
  Printf("%d\n", argc);
  for (i = 0; i < argc; i++) {
    Printf("%s\n", argv[i]);
  }
  Printf("Hello world\n");
}
