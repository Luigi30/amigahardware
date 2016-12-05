#ifndef BLIT_H
#define BLIT_H

#include <stdio.h>
#include <stdlib.h>

#include <exec/memory.h>
#include <exec/types.h>
#include <graphics/gfx.h>

#include <clib/exec_protos.h>

#include "amiga.h"
#include "serial.h"

struct Bob_Sprite {
	int position_x;
	int position_y;
	int width;
	int height;
	int bitplanes; //number of bitplanes in the graphics data
	PLANEPTR graphics[5]; //the graphics data
	
	PLANEPTR background[5]; //buffer for saved background data up to 5 bitplanes
	int background_x; //where did this background come from?
	int background_y;
};

void B_ClearBitplane(PLANEPTR bitplane, int width, int height);
void B_Blit(PLANEPTR destination, int destX, int destY, APTR source, int srcX, int srcY, int sizeW, int sizeH, PLANEPTR oldBackground);
PLANEPTR B_SaveBackground(PLANEPTR bitplane, int x, int y, int width, int height);
void B_RestoreBackground(PLANEPTR destination, int destX, int destY, PLANEPTR source, int srcX, int srcY, int sizeW, int sizeH);
void B_CheckBobBackground(PLANEPTR bitplanes[], struct Bob_Sprite *bob);
void B_BlitTileRow(PLANEPTR bitplanes[], UBYTE tileIndices[], PLANEPTR bgTileGraphics[], int row);
void B_PlaceBob(PLANEPTR bitplanes[], struct Bob_Sprite *bob);
struct Bob_Sprite *B_AllocateBobSprite();
void B_FreeBobSprite(struct Bob_Sprite *bob);

#endif
