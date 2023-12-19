#include "usertraps.h"
#include "files_shared.h"

void main(int argc, char *argv[]) {
  char *file = "test2g76.txt";
  char *string = "Lets make this a really long string so that we can go through and test the functionality of what we are doing!";
  char *array[4096];
  int testfail = 0;
  int handle;

  //file open
  if((handle = file_open(file, "rw")) == FILE_FAIL) {
    Printf("Error opening in read write mode!\n");
    testfail++;
  }
  if((handle = file_open(file, "rw")) == FILE_FAIL) {
    Printf("Cant open two of the same file!\n");
    testfail++;
  }

  //file close
  if(file_close(handle) == FILE_FAIL) {
    Printf("Error closing file!\n");
    testfail++;
  }
  if((handle = file_open(file, "w")) == FILE_FAIL) {
    Printf("Error opening in write write mode!\n");
    testfail++;
  }
  //file write
  if(file_write(handle, string, 110) == FILE_FAIL); {
    Printf("unable to write to file!\n");
    testfail++;
  }
  if(file_close(handle) == FILE_FAIL) {
    Printf("Error closing file!\n");
    testfail++;
  }

  if((handle = file_open(file, "r")) == FILE_FAIL) {
    Printf("Error opening in read mode!\n");
    testfail++;
  }
  //file seek set
  if(file_seek(handle,0,FILE_SEEK_SET) == FILE_FAIL) {
    Printf("Error seek to begin!\n");
    testfail++;
  }
  //file read
  if(file_read(handle,array,110) == FILE_FAIL) {
    Printf("Failed to read!\n");
    testfail++;
  }
  Printf("read from file: %s",array);

  //file seek cur
  if(file_seek(handle,-(110/2),FILE_SEEK_CUR) == FILE_FAIL) {
    Printf("Error seek to -1/2 from end!\n");
    testfail++;
  }
  //file read
  if(file_read(handle,array,110/2) == FILE_FAIL) {		//think this should work what are the chances i didnt write an odd amount of chars
    Printf("Failed to read!\n");
    testfail++;
  }
  Printf("read from file: %s",array);
  
  //file seek set
  if(file_seek(handle,-110,FILE_SEEK_END) == FILE_FAIL) {
    Printf("Error seek to begin!\n");
    testfail++;
  }
  //file read
  if(file_read(handle,array,110) == FILE_FAIL) {
    Printf("Failed to read!\n");
    testfail++;
  }
  Printf("read from file: %s",array);
  
  
  if(file_delete(file) == FILE_FAIL) {
    Printf("failed to delete file!\n");
    testfail++;
  }

  Printf("You failed %d tests!\n", testfail);
  
  if(!testfail) {
    Printf("correct permissions!!!\n\n");
  }
  else {
    Printf("permissions not right!\n");
  }

}
