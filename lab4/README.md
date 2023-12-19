# Lab 4 Group 76
Kevin Shi
Michael Kolb
Sam Butchko 

Lab 4 implements basic memory manipulations in DLXOS. One-level paging is used to store and organize up to 4kB 
of memory. Fork() and copy on write are also implemented in order to spawn multiple processes.  

Lab 4 implements basic one-level paging given a virtual memory size of 1MB, 2MB physical memory size, and a page size of 4KB. This lab also demonstrates the functionality 
of the system call fork(), which is used to spawn child processes from a parent, and is implemented using the copy-on-write algorithm. Lastly, dynamic memory allocation functions such as malloc() and free() are also implemented using the buddy allocation algorithm


## Instructions 

1. Navigate to ```one-level``` directory for this question The code for this question is written in memory.c and process.c, compile the os by typing ```make```to so that it now contains paging functionality. This will be further tested in the next step.

2. After completing building DLXOS as indicated in step 1, navigate to one-level/apps/examples/ where all the testing code for q2 is found. Compile the testing code by running ```make``` and running the program by typing ```make run```. For each of the tests, the PID is printed along with the process name, and the args (test number and semaphore handle in that order). Test 2 was put last, because it results in an illegal access, which is to be expected from accessing beyond the bounds of the allocate virtual memory.

3. For this step navigate to the ```fork``` directory. All the code for the fork function is found in process.c in ProcessRealFork, and the ROP handler is found in memory.c. Update DLXOS by running ```make``` in the ```os``` directory.

4. The testing code for this question is found in ```fork/apps/fork_test```. Run ```make``` followed by ```make run``` to execute the program. The program should output the parent and child pagetable entries once after the parent has forked, and once after a ROP exception is thrown.

5. This step contains code for malloc and free, and are located in ```heap-mgmt/os```. Compile the os by navigating to that directory, and navigate to ```apps/heap-test```. Compile and run the program by running ```make``` and ```make run```. The debug messages for malloc, and free should be printed following each call.

