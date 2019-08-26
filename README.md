# Cellular-Automaton
C Program implementing pipes, multi-threading, thread synchronization, and bash scripting.

## Objectives
This assignment is different from the last one in the sense that you get a complete working program
to work with, and need to add functionality. The objectives of this assignment are for you to

• Get some practice at modifying an existing code base, and in particular develop a “feel”
for how much of the existing code base you must understand in detail and how much only
requires very superficial, global understanding;

• Use external libraries in a C project;

• Use named pipes to establish communications between a bash script and processes running
a C executable.

• Implement different forms of multithreaded;

• Synchronize access to shared resources using either a single, global mutex lock or multiple
mutex locks.

## The problem

### Multithread the program
The simplest way to do this is to assign a horizontal band of the grid to each process. Of course,
depending on the number of threads and the height of your grid, the last computing thread may not
have the same number of rows to process as the other threads.

Because we read the data into the “current” version of the grid and write the results into the
“next” copy of the grid, and no two threads ever write at the same location in the “next” grid, there
is no synchronization problem between computation threads.

On the other hand, there is a problem with the rendering thread (which is the main thread of
the process, once it engages in the call to glutMainLoop()). In theory, there could be a race
condition on the nextGrid and nextGrid2D variables.

So, you are going to synchronize access to these variables by adding a mutex lock that will
protect critical section regarding the two variables.

### Keep the threads in phase
