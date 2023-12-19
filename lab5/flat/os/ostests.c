#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"

void RunOSTests() {
  // STUDENT: run any os-level tests here
  //  DfsModuleInit();

  int handle;
  char array[50];
  char entry1[] = "chevy";
  char entry2[] = "ford";
  char entry3[] = "honda";
  //char entry4[] = "acura";	//didnt end up using
  char entry5[] = "mclaren";
  //test case 1 write block assigned sets of bytes

  printf("Test case 1, block aligned bytes\n");
  if(DfsInodeFilenameExists("test1g76") == DFS_FAIL) {
    printf("\ntest1g76 hasn't been created\n");
    handle = DfsInodeOpen("test1g76");

    DfsInodeWriteBytes(handle, entry1, 1000, dstrlen(entry1) + 1);
    DfsInodeWriteBytes(handle, entry2, 1050, dstrlen(entry2) + 1);

    DfsInodeReadBytes(handle, array, 1000, dstrlen(entry1) + 1);
    printf("From file: %d read: %s\n", handle, array);
    DfsInodeWriteBytes(handle, entry5, 1000, dstrlen(entry5) + 1);
    DfsInodeReadBytes(handle,array,1000, dstrlen(entry5) + 1);
    printf("From file: %d read: %s\n", handle, array);

    DfsInodeWriteBytes(handle, entry3, 12000, dstrlen(entry3) + 1);
    printf("\nOS filesystem test 1 done\n");
    return;
  } 
  else {
    handle = DfsInodeOpen("test1g76");
  }
  printf("test\n");
  if(DfsInodeFilenameExists("test1g76") == handle) {
    printf("File: %d is there\n",handle);
  }

  DfsInodeReadBytes(handle, array, 1000, dstrlen(entry5) + 1);
  //printf("From file: %d read: %s\n", handle, array);

  DfsInodeReadBytes(handle, array, 1050,dstrlen(entry2) + 1);
  //printf("From file: %d read: %s\n", handle, array);

  DfsInodeReadBytes(handle, array, 12000,dstrlen(entry3) + 1);
  //printf("From file: %d read: %s\n", handle, array);

  printf("\nHopefully this is everything.Deleting now!\n");
  DfsInodeDelete(handle);

  if(DfsInodeFilenameExists(handle) == DFS_FAIL) {
    printf("\n\nDelete worked!\n\n");
  }
}

