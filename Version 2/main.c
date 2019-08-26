//
//  main.c
//  Cellular Automaton
//
//  Nathan Larson 2018-04-22
//

/*--------------------------------------------------------------------------+
|	A graphic front end for a grid+state simulation.						|
|																			|
|	This application simply creates a glut window with a pane to display	|
|	a colored grid and the other to display some state information.			|
|	Sets up callback functions to handle menu, mouse and keyboard events.	|
|																			|
|	Current keyboard controls:												|
|																			|
|		- 'ESC' --> exit the application									|
|		- space bar --> resets the grid										|
|																			|
|		- 'c' --> toggle color mode on/off									|
|		- 'b' --> toggles color mode off/on									|
|		- 'l' --> toggles on/off grid line rendering						|
|																			|
|		- '+' --> increase simulation speed									|
|		- '-' --> reduce simulation speed									|
|																			|
|		- '1' --> apply Rule 1 (Conway's classical Game of Life: B3/S23)	|
|		- '2' --> apply Rule 2 (Coral: B3/S45678)							|
|		- '3' --> apply Rule 3 (Amoeba: B357/S1358)							|
|		- '4' --> apply Rule 4 (Maze: B3/S12345)							|
|																			|
+--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
//
#include "gl_frontEnd.h"

//==================================================================================
//	Custom data types
//==================================================================================
typedef struct ThreadInfo
{
	pthread_t 	threadID;
	int 		index;
	//
	//	whatever other input or output data may be needed
	//
} ThreadInfo;


//==================================================================================
//	Function prototypes
//==================================================================================
void displayGridPane(void);
void displayStatePane(void);
void initializeApplication(void);
void* threadFunc(void*);
unsigned int cellNewState(unsigned int i, unsigned int j);
void oneCellGeneration(int i, int j);
void* pipeServerThread(void*);

//==================================================================================
//	Precompiler #define to let us specify how things should be handled at the
//	border of the frame
//==================================================================================

#define FRAME_FIXED			-1	//	the one demo-ed in class
#define FRAME_DEAD			0	//	cell borders are kept dead
#define FRAME_RANDOM		1	//	new random values are generated at each generation
#define FRAME_CLIPPED		2	//	same rule as elsewhere, with clipping to stay within bounds
#define FRAME_WRAP			3	//	same rule as elsewhere, with wrapping around at edges

//	Pick one value for FRAME_BEHAVIOR
#define FRAME_BEHAVIOR	FRAME_DEAD

//==================================================================================
//	Application-level global variables
//==================================================================================

extern const int GRID_PANE, STATE_PANE;
extern int gMainWindow, gSubwindow[2];

//	The state grid and its dimensions.  We now have two copies of the grid:
//		- currentGrid is the one displayed in the graphic front end
//		- nextGrid is the grid that stores the next generation of cell
//			states, as computed by our threads.
int* currentGrid;
int** currentGrid2D;

pthread_mutex_t* gridMutex;
pthread_mutex_t** gridMutex2D;

int numRows, numCols;

//	the number of live threads (that haven't terminated yet)
int maxThreadCount;

unsigned int numLiveThreads = 0;

unsigned int rule = GAME_OF_LIFE_RULE;

unsigned int colorMode = 0;

int sleepTimer = 100000;

//------------------------------
//	Threads and synchronization
//	Reminder of all declarations and function calls
//------------------------------

//int err = pthread_create(pthread_t*, NULL, threadFunc, ThreadInfo*);
//int pthread_join(pthread_t , void**);
//pthread_mutex_lock(&myLock);
//pthread_mutex_unlock(&myLock);


void displayGridPane(void)
{
	//	This is OpenGL/glut magic.
	glutSetWindow(gSubwindow[GRID_PANE]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//---------------------------------------------------------
	//	This is the call that makes OpenGL render the grid.
	//
	//---------------------------------------------------------
	drawGrid(currentGrid2D, numRows, numCols);
	
	//	This is OpenGL/glut magic.
	glutSwapBuffers();
	
	glutSetWindow(gMainWindow);
}

void displayStatePane(void)
{
	//	This is OpenGL/glut magic.
	glutSetWindow(gSubwindow[STATE_PANE]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//---------------------------------------------------------
	//	This is the call that makes OpenGL render information
	//	about the state of the simulation.
	//
	//---------------------------------------------------------
	drawState(numLiveThreads, maxThreadCount);
	drawRule(rule);
	drawSleepTimer();
	drawTitle();
	
	
	//	This is OpenGL/glut magic.
	glutSwapBuffers();
	
	glutSetWindow(gMainWindow);
}

/*
 * Main function
 */
int main(int argc, char** argv)
{
	if(argc < 3 || argc > 4)	// if there are too little or too many parameters, print error and exit
	{
		printf("\n\nMust enter correct format(s): \t./cell 'rows' 'columns' 'max thread count'\n\t\t\t./cell 'rows' 'columns'\n");
		exit(0);
	}
	else
	{
		if(argc == 4)			// if there are 4 parameters, set the corresponding values for rows, columns, and max thread count
		{
			sscanf(argv[1], "%d", &numRows);
			sscanf(argv[2], "%d", &numCols);
			sscanf(argv[3], "%d", &maxThreadCount);
		}
		else if(argc == 3)		// else if there are 3 parameters, set the corresponding values for rows and columns, then set max thread count to row count
		{
			sscanf(argv[1], "%d", &numRows);
			sscanf(argv[2], "%d", &numCols);
			maxThreadCount = numRows;
		}

		if((numRows < 5) || (numCols < 5))		// if the rows or columns are less than 5, print error and exit
		{
			printf("\n\nRow and Column count must be larger than 5.\n\n");
			exit(0);
		}
		if(maxThreadCount > numRows)			// if the max thread count is larger than the number of rows, print error and exit
		{
			printf("\n\nThread count cannot be greater than number of rows.\n\n");
			exit(0);
		}
	}

	// creating the server thread for the named pipe
	pthread_t serverID;
	int serverCode = pthread_create(&serverID, NULL, pipeServerThread, NULL);
	if( serverCode != 0)
	{
		printf ("could not pthread_create server thread.\n");
		exit(0);
	}


	//	This takes care of initializing glut and the GUI.
	initializeFrontEnd(argc, argv, displayGridPane, displayStatePane);
	
	//	Now we can do application-level initialization
	initializeApplication();

	//	Now would be the place & time to create mutex locks and threads

	// declare the array of ThreadInfo structs
	ThreadInfo threads[maxThreadCount];

	int errCode;		
	for(int i = 0; i < maxThreadCount; i++)		// for loop to loop through and create determined number of threads
	{

		// assign the values to the indexed ThreadInfo struct 
		threads[i].index = i;					// index of given thread in ThreadInfo array

		// create the pthread
		errCode = pthread_create(&threads[i].threadID, NULL, threadFunc, &threads[i]);

		// increment the number of live threads
		numLiveThreads++;

		// if the errCode is nonzero, then the pthread was not created. print error and exit
		if(errCode != 0)
		{
			printf ("could not pthread_create thread %d. %d/%s\n",
					 i, errCode, strerror(errCode));
			exit(0);
		}
	}

	//	Now we enter the main loop of the program and to a large extend
	//	"lose control" over its execution.  The callback functions that 
	//	we set up earlier will be called when the corresponding event
	//	occurs
	glutMainLoop();
	
	//	this code is never reached because we only leave the glut main
	//	loop through an exit call.
	free(currentGrid2D);
	free(currentGrid);

	free(gridMutex2D);
	free(gridMutex);
	
	
	//	This will never be executed (the exit point will be in one of the
	//	call back functions).
	return 0;
}

/*
 * This function is called in a separate thread, handling the communication between the user and program
 * using a named pipe.
 * The user input is given from a bash script, which then send the command over a named pipe handled by this function.
 */
void* pipeServerThread(void *unused)
{
	FILE *fp;
	char path[] = "/tmp/prog04pipe";
	char readbuf[80];

	umask(0);
	mknod(path, S_IFIFO|0666, 0);

	//printf("Server listening for commands...");
	while(1)
	{
		fp = fopen(path, "r");
		fgets(readbuf, 80, fp);
		commandHandler(readbuf);
		fclose(fp);
	}
	return NULL;
}

/*
 * Initialize application
 *		-Allocate memory
 *		-Seed "random" number generator
 *		-Initialize grid values
 */
void initializeApplication(void)
{
    //  Allocate 1D grids
    //--------------------
    currentGrid = (int*) malloc(numRows*numCols*sizeof(int));
    gridMutex = (pthread_mutex_t*) malloc(numRows*numCols*sizeof(pthread_mutex_t));

    //  Scaffold 2D arrays on top of the 1D arrays
    //---------------------------------------------
    currentGrid2D = (int**) malloc(numRows*sizeof(int*));
    gridMutex2D = (pthread_mutex_t**) malloc(numRows*sizeof(pthread_mutex_t*));
    
    currentGrid2D[0] = currentGrid;
    gridMutex2D[0] = gridMutex;
    for (int i=1; i<numRows; i++)
    {
        currentGrid2D[i] = currentGrid2D[i-1] + numCols;
        gridMutex2D[i] = gridMutex2D[i-1] + numCols;
    }

    // initialize all of the locks in the 2D array of mutex locks
    for(int i = 0; i < numRows; i++)
    {
    	for(int j = 0; j < numCols; j++)
    	{
    		pthread_mutex_init(&gridMutex2D[i][j], NULL);
    	}
    }
	
	//	seed the pseudo-random generator
	srand((unsigned int) time(NULL));
	
	resetGrid();
}

/*
 * Acts as the main function for the thread(s)
 */
void* threadFunc(void* arg)
{
	ThreadInfo* info = (ThreadInfo *) arg;

	// while loop to continue calculating the next generation of cells 
	while(1)
	{
		// generate a random index on the x and y axis (row and column)
		int randomCol = rand() % (numCols);
		int randomRow = rand() % (numRows);
		// get the mutex lock for the 3x3 square around the selected cell
		if (randomRow>0 && randomRow<numRows-1 && randomCol>0 && randomCol<numCols-1)
		{
			pthread_mutex_lock(&gridMutex2D[randomRow - 1][randomCol - 1]);
			pthread_mutex_lock(&gridMutex2D[randomRow - 1][randomCol]);
			pthread_mutex_lock(&gridMutex2D[randomRow - 1][randomCol + 1]);
			pthread_mutex_lock(&gridMutex2D[randomRow][randomCol - 1]);
			pthread_mutex_lock(&gridMutex2D[randomRow][randomCol]);
			pthread_mutex_lock(&gridMutex2D[randomRow][randomCol + 1]);
			pthread_mutex_lock(&gridMutex2D[randomRow + 1][randomCol - 1]);
			pthread_mutex_lock(&gridMutex2D[randomRow + 1][randomCol]);
			pthread_mutex_lock(&gridMutex2D[randomRow + 1][randomCol + 1]);
			//printf("Thread %d acquired all locks.\n", info->index);
			oneCellGeneration(randomRow, randomCol);
			pthread_mutex_unlock(&gridMutex2D[randomRow - 1][randomCol - 1]);
			pthread_mutex_unlock(&gridMutex2D[randomRow - 1][randomCol]);
			pthread_mutex_unlock(&gridMutex2D[randomRow - 1][randomCol + 1]);
			pthread_mutex_unlock(&gridMutex2D[randomRow][randomCol - 1]);
			pthread_mutex_unlock(&gridMutex2D[randomRow][randomCol]);
			pthread_mutex_unlock(&gridMutex2D[randomRow][randomCol + 1]);
			pthread_mutex_unlock(&gridMutex2D[randomRow + 1][randomCol - 1]);
			pthread_mutex_unlock(&gridMutex2D[randomRow + 1][randomCol]);
			pthread_mutex_unlock(&gridMutex2D[randomRow + 1][randomCol + 1]);
		}
		else
		{
			pthread_mutex_lock(&gridMutex2D[randomRow][randomCol]);
			//printf("Thread %d acquired single lock.\n", info->index);
			oneCellGeneration(randomRow, randomCol);
			pthread_mutex_unlock(&gridMutex2D[randomRow][randomCol]);
		}

		usleep(sleepTimer);
		
	}
	return NULL;
}


void resetGrid(void)
{
	for (int i=0; i<numRows; i++)
	{
		for (int j=0; j<numCols; j++)
		{
			pthread_mutex_lock(&gridMutex2D[i][j]);
			currentGrid2D[i][j] = rand() % 2;
			pthread_mutex_unlock(&gridMutex2D[i][j]);
		}
	}
}

/*
 * This function generates one cell indexed by the given parameters.
 */
void oneCellGeneration(int i, int j)
{
	unsigned int newState = cellNewState(i, j);

	//	In black and white mode, only alive/dead matters
	//	Dead is dead in any mode
	if (colorMode == 0 || newState == 0)
	{
		currentGrid2D[i][j] = newState;
	}
	//	in color mode, color reflext the "age" of a live cell
	else
	{
		//	Any cell that has not yet reached the "very old cell"
		//	stage simply got one generation older
		if (currentGrid2D[i][j] < NB_COLORS-1)
			currentGrid2D[i][j] = currentGrid2D[i][j] + 1;
		//	An old cell remains old until it dies

	}
}

unsigned int cellNewState(unsigned int i, unsigned int j)
{
	//	First count the number of neighbors that are alive
	int count = 0;

	//	Away from the border, we simply count how many among the cell's
	//	eight neighbors are alive (cell state > 0)
	if (i>0 && i<numRows-1 && j>0 && j<numCols-1)
	{
		//	remember that in C, (x == val) is either 1 or 0
		count = (currentGrid2D[i-1][j-1] != 0) +
				(currentGrid2D[i-1][j] != 0) +
				(currentGrid2D[i-1][j+1] != 0)  +
				(currentGrid2D[i][j-1] != 0)  +
				(currentGrid2D[i][j+1] != 0)  +
				(currentGrid2D[i+1][j-1] != 0)  +
				(currentGrid2D[i+1][j] != 0)  +
				(currentGrid2D[i+1][j+1] != 0);
	}
	//	on the border of the frame...
	else
	{
		#if FRAME_BEHAVIOR == FRAME_DEAD
		
			//	Hack to force death of a cell
			count = -1;
		
		#elif FRAME_BEHAVIOR == FRAME_RANDOM
		
			count = rand() % 9;
		
		#elif FRAME_BEHAVIOR == FRAME_CLIPPED
	
			if (i>0)
			{
				if (j>0 && currentGrid2D[i-1][j-1] != 0)
					count++;
				if (currentGrid2D[i-1][j] != 0)
					count++;
				if (j<numCols-1 && currentGrid2D[i-1][j+1] != 0)
					count++;
			}

			if (j>0 && currentGrid2D[i][j-1] != 0)
				count++;
			if (j<numCols-1 && currentGrid2D[i][j+1] != 0)
				count++;

			if (i<numRows-1)
			{
				if (j>0 && currentGrid2D[i+1][j-1] != 0)
					count++;
				if (currentGrid2D[i+1][j] != 0)
					count++;
				if (j<numCols-1 && currentGrid2D[i+1][j+1] != 0)
					count++;
			}
			
	
		#elif FRAME_BEHAVIOR == FRAME_WRAPPED
	
			unsigned int 	iM1 = (i+numRows-1)%numRows,
							iP1 = (i+1)%numRows,
							jM1 = (j+numCols-1)%numCols,
							jP1 = (j+1)%numCols;
			count = currentGrid2D[iM1][jM1] != 0 +
					currentGrid2D[iM1][j] != 0 +
					currentGrid2D[iM1][jP1] != 0  +
					currentGrid2D[i][jM1] != 0  +
					currentGrid2D[i][jP1] != 0  +
					currentGrid2D[iP1][jM1] != 0  +
					currentGrid2D[iP1][j] != 0  +
					currentGrid2D[iP1][jP1] != 0 ;

		#else
			#error undefined frame behavior
		#endif
		
	}	//	end of else case (on border)
	
	//	Next apply the cellular automaton rule
	//----------------------------------------------------
	//	by default, the grid square is going to be empty/dead
	unsigned int newState = 0;
	
	//	unless....
	
	switch (rule)
	{
		//	Rule 1 (Conway's classical Game of Life: B3/S23)
		case GAME_OF_LIFE_RULE:

			//	if the cell is currently occupied by a live cell, look at "Stay alive rule"
			if (currentGrid2D[i][j] != 0)
			{
				if (count == 3 || count == 2)
					newState = 1;
			}
			//	if the grid square is currently empty, look at the "Birth of a new cell" rule
			else
			{
				if (count == 3)
					newState = 1;
			}
			break;
	
		//	Rule 2 (Coral Growth: B3/S45678)
		case CORAL_GROWTH_RULE:

			//	if the cell is currently occupied by a live cell, look at "Stay alive rule"
			if (currentGrid2D[i][j] != 0)
			{
				if (count > 3)
					newState = 1;
			}
			//	if the grid square is currently empty, look at the "Birth of a new cell" rule
			else
			{
				if (count == 3)
					newState = 1;
			}
			break;
			
		//	Rule 3 (Amoeba: B357/S1358)
		case AMOEBA_RULE:

			//	if the cell is currently occupied by a live cell, look at "Stay alive rule"
			if (currentGrid2D[i][j] != 0)
			{
				if (count == 1 || count == 3 || count == 5 || count == 8)
					newState = 1;
			}
			//	if the grid square is currently empty, look at the "Birth of a new cell" rule
			else
			{
				if (count == 1 || count == 3 || count == 5 || count == 8)
					newState = 1;
			}
			break;
		
		//	Rule 4 (Maze: B3/S12345)							|
		case MAZE_RULE:

			//	if the cell is currently occupied by a live cell, look at "Stay alive rule"
			if (currentGrid2D[i][j] != 0)
			{
				if (count >= 1 && count <= 5)
					newState = 1;
			}
			//	if the grid square is currently empty, look at the "Birth of a new cell" rule
			else
			{
				if (count == 3)
					newState = 1;
			}
			break;

			break;
		
		default:
			printf("Invalid rule number\n");
			exit(5);
	}

	return newState;
}
