#ifndef BOB_H
#define BOB_H

#include <exec/memory.h>
#include <exec/types.h>
#include <graphics/gfx.h>

#include <clib/exec_protos.h>

#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct BlitterObject {
	int position_x;
	int position_y;
	int width;
	int height;
	int bitplaneNum; //number of bitplanes in the graphics data
	PLANEPTR bitmap; //interleaved. bitmap should be width x height*bitplaneNum pixels
	
	/* background */
	PLANEPTR background; //buffer for saved background pixels
	int background_x; //what screen coordinates did this background come from?
	int background_y;
	
	PLANEPTR mask; //unused so far
} BlitterObject;

BlitterObject *BOB_Allocate();
void BOB_Free(BlitterObject *bob);

#endif
