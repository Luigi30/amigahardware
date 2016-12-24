#ifndef MAIN_H
#define MAIN_H

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

#include "globals.h"
#include "game/game.h"
#include "image/loader.h"
#include "sprites/sprites.h"
#include "serial.h"
#include "blit.h"
#include "input.h"

struct GfxBase *GraphicsBase;

PLANEPTR CopperPtr_Bitplane1;
PLANEPTR CopperPtr_Bitplane2;
PLANEPTR CopperPtr_Bitplane3;
PLANEPTR CopperPtr_Bitplane4;

struct Bob_Sprite *ship;

void AssumeDirectControl();
void ReleaseSystem();

void B_Blit32x32(PLANEPTR destination, APTR source, int x, int y);

extern int ReadJoystick();

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
