# Lab 3

This lab demonstrates the implementation and the usage of the DLXOS mailbox
system, and also replaces the process scheduling algorithm in dlxos with the
BSD algorithm instead of using the inefficient-but-fair round-robin algorithm.

## Instructions
1. Build:
You will want to look at mbox.c in the os folder for question 1 and then follow the next steps.
To build this program you will need to run the ```make``` command inside the
os folder. To test the functionality of the mailbox API, go to apps/example,
and type ```make``` to build the example program, and type ```make run``` to
execute the program. 
2. For Q2. Compile the os by typing ```make``` in the os directory once again,
and type ```make``` in apps/q2 to build the program. This will compile
makeprocs, producers, and consumers altogether. Afterwards, type ```make
run``` to run the program.
3. Code for Q3 is located inside of the first ProcessSchedule definition in
os/process.c. Uncomment the second definition since that is for the BSD algorithm that we
had to implement in q4. Build the os by typing ```make```, and to test this,
go to apps/example/makeprocs.c, and for each process_create call, replace the
third parameter with 1 instead of 0. This will set pinfo to 1 to enable CPU
runtime statistics.
4 This step is for q4 and q5 combined. To run the BSD process scheduling
algorithm. Comment the first ProcessSchedule function definition in
os/process.c, and uncomment the second one. Build the os by typing ```make```
in the os directory. To test this, navigate to apps/prio_test, and type
```make``` to build the program and type ```make run``` to run.


## Issues
For the mailbox solution of q2 we kept running into issues with the inputs being hotswapped, for some reason we will get the right output then on one we will get CO -> S + S and it breaks the code we have struggled on how best to fix it.

For q4, when we ran prio_test, we were not able to get any output after the
child processes were spawned. This could be a possible issue with our
implementation of the BSD scheduling algorithm.
