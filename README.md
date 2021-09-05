
# Project 1: Operating System Monitoring Tool

## 

Operating system monitoring tool runs in user space, outside the kernel. 

An Unix utility to monitor the system and creates a summarized report for the user. (Shell flavor)

Project supervisor: Malensek @USF 

```sh

Hostname: Rozita | Kernel Version: 5.4.15
CPU: AMD EPYC Processor (with IBPB), Processing Units: 2
Uptime: 3 days, 23 hours, 51 minutes, 19 seconds

Load Average (1/5/15 min): 0.05 0.02 0.00
CPU Usage:    [######--------------] 30.0%
Memory Usage: [##################--] 90.6% (14.5 / 16.0 GB)

Tasks: 101 total
2 running, 0 waiting, 98 sleeping, 1 stopped, 0 zombie

      PID |                 Task Name |        State |            User
----------+---------------------------+--------------+-----------------
     9783 |                      sshd |      running |            root
    11642 |                       top |      stopped |            Rozita
    12701 |                 inspector |      runn

```

## Introduction

To get this information, we will use the proc, the process information pseudo-filesystem. While there are other ways to get the information displayed above, we are restricted to using proc in this assignment. There are two great resources for finding out what information is available in proc:

Simply cd /proc in the shell and then run ls to view the files. 
You’ll see process IDs and several other virtual files that are updated dynamically with system information. Each line shown above in the process listing corresponds to a numbered directory in /proc.

Check out the man page: man procfs. (If this doesn’t work, you probably need to install the man pages: pacman -Sy man-pages). The manual has a complete description of every file and directory stored under /proc.
For a quick example, try running cat /proc/uptime. You’ll see the number of seconds the system has been running printed to the terminal.

We will working with:

* The open(), read(), and close() system calls for reading file data
* Tokenizing text files
* opendir and readdir functions for listing directory contents
* Argument parsing with getopt
* Load averages, calculating CPU usage, and Linux tasks
* Text-based UIs with ncurses

The program supports a few command line options. We’ll let the program do the talking by printing usage information (-h option):

```bash
[magical-unicorn:~/P1-malensek]$ ./inspector -h
Usage: ./inspector [-ho] [-i interval] [-p procfs_dir]

Options:
    * -h              Display help/usage information
    * -i interval     Set the update interval (default: 1000ms)
    * -p procfs_dir   Set the expected procfs mount point (default: /proc)
    * -o              Operate in one-shot mode (no curses or live updates)
```

Pay particular attention to the -p flag. This allows us to change the directory where proc is mounted (/proc by default). We will use this option to test your code with our own pre-populated copy of proc.


## Populating the Output

The main responsibility in this project is implementing a variety of procfs-related functions (prefixed pfs_*) to retrieve the required information.

The UI for the program was written using the ncurses library – assuming your pfs_* functions work properly, you shouldn’t need to worry about modifying the UI (although you are welcome to do so). In ‘one-shot’ mode, the simple UI is disabled and printed directly to the terminal instead.

Here’s some tips to guide your implementation:

Remember to close the files/directories you open! If too many file descriptors are left open, subsequent open calls will fail.
Truncate strings that are too long to display (applicable to the process and user names). Otherwise, you can run into memory issues.
Use the total and available memory when reporting memory usage.
If you need to store a list of items that you don’t know the size of ahead of time, realloc can be a good option.


## Features

* Read hostname : /proc/sys/kernel/hostname
* Read Kernel Version : /proc/version
* Read CPU model : /proc/cpuinfo
* Read CPU unit : /proc/stat
* Read Uptime : /proc/uptime
* Read Load Average : /proc/loadavg
* Read CPU Usage : /proc/state
* Read Memory Usage : /proc/meminfo
* Read Task : /proc/<number folder>/status

### Uptime

When calculating uptime, don’t report years, days, or hours if their respective values are 0. If a machine has just booted up, you’ll display Uptime: 0 minutes, 42 seconds, for example (note that we’re only showing the minutes and seconds fields). The fields you need to support are:

* Years
* Days
* Hours
* Minutes
* Seconds
* CPU Usage

CPU usage is calculated by sampling over a period of time, i.e., the CPU was active for 70% of one second. You will record the CPU usage, sleep for one second, and then get a second reading to determine the usage percentage. The CPU metrics in /proc/stat will add up to 100% because idle time is included. You’ll need to track idle time separately, so the calculation will look something like:

```equ
1 - ( (idle2 - idle1) / (total2 - total1))
```

If the CPU usage percentage is NaN (not a number), or you encounter errors performing the calculation, report 0%. Since this calculation requires two samples, your function should take the previous sample as one of its arguments.

### Process States

You should support all of the process states listed in the proc man pages. Additionally, for our purposes, we will consider ‘idle’ processes equivalent to ‘sleeping.’ To build the “running, waiting, sleeping, …” output, the specific state flags you’ll be interested in are R, S, I, D, Z, T, and t.

Idle and sleeping processes are not shown in the program output.

### UIDs
You’ll notice that proc doesn’t contain information about the username associated with running processes, but it does provide their ID numbers (UIDs). To map UIDs to usernames, you will need to parse the contents of /etc/passwd. While there are functions that will do this for you, such as getpwuid, you are required to build your own lookup functionality. (Note: getpwuid can seemingly leak memory in certain situations, so we’re avoiding it here).

## Implementation Restrictions
Restrictions: you may use any standard C library functionality. External libraries are not allowed unless permission is granted in advance. Your code must compile and run on your VM set up with Arch Linux as described in class – failure to do so will receive a grade of 0.

While there are several ways to retrieve the system information displayed by your project, you must retrieve the data from /proc only.

One of the major components of this assignment is reading and parsing text files. To read the files, you are required to use the read system call instead of the fancier C library functions like fgets, getline, scanf, etc. You also shouldn’t use getpwuid or strtok.

Rationale: we’re using read here to get familiar with how I/O works at a lower level. You will need to be able to understand read for subsequent assignments. As for strtok, it has several pitfalls (including not being reentrant or thread safe) that make it a bad choice. You can either use strsep or the next_token implementation provided in class.

Failure to follow these guidelines will result in severe deductions (at least 10 points) or a 0.

## Testing The Code
Check your code against the provided test cases. You should make sure your code runs on your Arch Linux VM. We’ll have interactive grading for projects, where you will demonstrate program functionality and walk through your logic.



## Run

To compile and run:

```bash
make
./inspector
```

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'
```
Ref: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-1.html 
