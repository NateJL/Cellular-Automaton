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

## Version 1: Multithreaded with a single mutex lock

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
Right now, there is a single thread in the process: the main thread which then dedicates itself as
rendering thread nonce it makes the call to glutMainLoop(). That thread is the one running
the call to compute the next generation.

Now that you are going to have multiple computation threads, how are you going to know
that the “next grid” has been completely computed and that we can now safely swap the grids and
launch the computation of the next generation? I will let you ponder this one on your own and
come up with your own solution.

### Speed-up and Slow-down
Finally, for this version to be complete, you need to figure out a way to enable the speedup and
slowdown key controls. This does not mean adjusting the delay of the callback to the timer function
(which should be set to a value around 10 to 30 milliseconds. Rather, you want to adjust the sleep
time between generations.

## Version 2: Randomized multithreaded with a grid of mutex locks

### An in-place implementation
This proceeds of the same idea of the randomized bubblesort implementation that we saw in class.
Here we are saying: Instead of a dumb execution in artificial strict order, row-by-row and columnby-column, why not let the update be more natural and random? In this version, each of the
computation threads would randomly select a cell to update and would perform the update in
place. In other words, there wouldn’t be anymore a “next grid,” as all calculations would be done
directly on the “current grid.” This also means that all cells won’t be anymore all at the same
generation. Some cells will “age” faster than others if they are lucky, or unlucky, depending how
you see it.

### Synchronization issues
In this new, randomized and in-place version of the cellular automaton algorithm, we now have a
race condition on all the cells of the grid. We could simply use the global lock that was used to
synchronize shared access with the rendering thread, but this would be boring.

Instead, we are going to use a 2D array of mutex locks, one for each cell of the grid (so,
essentially, we replace one grid, the “next grid,” by another: the array of mutex locks). In order to
update the sate of one cell, the computing thread will need to acquire the nine locks of the 3 × 3
square centered at that cell.

It is really important that these locks are always acquired in the same order2
. Here, because all
threads run the same code, this should not be an issue. Let’s say that you acquire the locks in order
to-to-bottom and left-to-right.

Now, synchronizing access to the entire grid with the rendering thread is even more critical
than before. Here again, I let you think about it an propose your own solution. I will just give you
a hint: Does this look somewhat like one of the classic problems?

## Part II: Bash Script

### Control the program from a bash script
The program of the handout uses a very rudimentary GUI to specify some of the parameters of
the cellular automaton. Using keystrokes, the user can select the cell reproduction rule, speedup
or slowdown the simulation, switch the display from black and white to color, and also terminate
execution.

What I want you do is to allow the user to perform the same adjustments to the cellular automaton by giving commands to the script. The purpose of this task in the assignment is not to improve
the user experience for this particular program but simply to add one more tool to your toolbox,
even if the task we use it for here is borderline silly/useless.

### What the script should do
The script should (not necessarily in this order):

• Get as arguments the width and height, and number of threads of the cellular automaton
process to launch;

• Launch a cellular automaton process, specifying the desired width and height;

• Setup a named pipe to communicate with the cellular automaton process, using the command
syntax defined in the next subsection.

### Syntax
• <b>rule</b> ¡rule number¿ selects the automaton’s cell reproduction rule. For example, rule 2
selects the “coral growth” rule.

• <b>color on</b> enables the color mode;

• <b>color off</b> disables the color mode (reverts to black and white);

• <b>speedup</b> increases the speed of the simulation;

• <b>slowdown</b> reduces the speed of the simulation;

• <b>end</b> terminates execution of the cellular automaton of the process and of the script.
Invalid commands (including rule selection commands with an invalid rule number) should be
properly rejected.

### Extra Credit
Modify your script so that it becomes a pure command interpreter and even the launching of a
cellular automaton process is done via a script command. This means that your script could now
launch and control multiple cellular automata. The command to launch a new cellular automaton
would be

• <b>cell</b> width height

Each new cellular automation process would receive an index (starting with the first process at an
index of 1) and display that index in its window’s title.

All other commands to be interpreted by the script should now be prefixed by the index of the
cellular automaton process to which they should be addressed:

• <b>3: speedup</b> means that Cellular automaton 3 should increase its speed;

• <b>2: end</b> means that Cellular automaton 2 should terminate execution;

• etc.
Invalid commands, including commands meant for a process that hasn’t been created or one that
has already terminated, should be rejected.
