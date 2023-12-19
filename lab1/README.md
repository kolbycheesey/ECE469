# Lab 1

This lab implements the GetPid command and tests it in a sample hello world program.

## Usage
1. Compile the user program by typing ```make``` in the apps directory.
2. Compile the operating system by typing ```make``` in the os directory.
3. Run the following command ```dlxsim -x /tmp/$USER/ee469/lab1/os/work/os.dlx.obj -a -u /tmp/$USER/ee469/lab1/apps/work/userprog.dlx.obj```. The Hello World program should now be running in the DLX operating system. It should print Hello World followed by a newline and the process ID. 31 should be displayed as the process ID.

