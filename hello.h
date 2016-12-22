#ifndef HELLO_H
#define HELLO_H

#define true 1
#define false 0

#define WIDTH 320
#define HEIGHT 208
#define DEPTH 5

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "image/loader.h"
#include "serial.h"
#include "blit.h"
#include "copperdefs.h"
#include "input.h"

#define LOWORD(l) ((WORD)(l))
#define HIWORD(l) ((WORD)(((LONG)(l) >> 16) & 0xFFFF))

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define NUM_BITPLANES 4

struct GfxBase *GraphicsBase;

struct ImageData *imageList;

PLANEPTR CopperPtr_Bitplane1;
PLANEPTR CopperPtr_Bitplane2;
PLANEPTR CopperPtr_Bitplane3;
PLANEPTR CopperPtr_Bitplane4;

//Double buffering!
PLANEPTR BPScreen1_Interleaved;
PLANEPTR BPStatusBar;
PLANEPTR CopperPtrs[5];

PLANEPTR tilemapBitmap;

struct Bob_Sprite *ship;

void AssumeDirectControl();
void ReleaseSystem();
int WaitForLMB();

void B_Blit32x32(PLANEPTR destination, APTR source, int x, int y);

extern int ReadJoystick();

struct SpriteCoordinates {
	UWORD x;
	UWORD y;
};

struct TiledBinaryHeader {
	UBYTE orientation;
	UBYTE staggerAxis;
	UBYTE staggerIndex;
	UWORD hexSideLength;
	UWORD mapWidth;
	UWORD mapHeight;
	UWORD tileWidth;
	UWORD tileHeight;
	UBYTE tileNumberBits;
	UBYTE rle;
	UWORD layerCount;
};

#endif
