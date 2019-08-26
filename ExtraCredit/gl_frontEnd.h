//
//  gl_frontEnd.h
//  Cellular Automaton
//
//  Nathan Larson 2018-04-22
//

#ifndef GL_FRONT_END_H
#define GL_FRONT_END_H


//------------------------------------------------------------------------------
//	Find out whether we are on Linux or macOS (sorry, Windows people)
//	and load the OpenGL & glut headers.
//	For the macOS, lets us choose which glut to use
//------------------------------------------------------------------------------
#if (defined(__dest_os) && (__dest_os == __mac_os )) || \
	defined(__APPLE_CPP__) || defined(__APPLE_CC__)
	//	Either use the Apple-provided---but deprecated---glut
	//	or the third-party freeglut install
	#if 0
		#include <GLUT/GLUT.h>
	#else
		#include <GL/freeglut.h>
		#include <GL/gl.h>
	#endif
#elif defined(linux)
	#include <GL/glut.h>
#else
	#error unknown OS
#endif


//-----------------------------------------------------------------------------
//	Custom data types
//-----------------------------------------------------------------------------


typedef enum ColorLabel {
	BLACK_COL = 0,
	WHITE_COL,
	BLUE_COL,
	GREEN_COL,
	YELLOW_COL,
	RED_COL,
	//
	NB_COLORS
} ColorLabel;

//	Rules of the automaton (in C, it's a lot more complicated than in
//	C++/Java/Python/Swift to define an easy-to-initialize data type storing
//	arrays of numbers.  So, in this program I hard-code my rules
#define GAME_OF_LIFE_RULE	1
#define CORAL_GROWTH_RULE	2
#define AMOEBA_RULE			3
#define MAZE_RULE			4


//-----------------------------------------------------------------------------
//	Function prototypes
//-----------------------------------------------------------------------------

void drawGrid(int**grid, unsigned int numRows, unsigned int numCols);
void drawState(unsigned int numLiveThreads, int maxThreadCount);
void drawRule(int currentRule);
void drawSleepTimer(void);
void drawTitle(int procID);
void initializeFrontEnd(int argc, char** argv, void (*gridCB)(void), void (*stateCB)(void));
void commandHandler(char* cmd);

//	Functions implemented in main.c but called byt the glut callback functions
void resetGrid(void);
void oneGeneration(void);


#endif // GL_FRONT_END_H

