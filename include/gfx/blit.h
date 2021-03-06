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
#include "gfx/bob.h"

void B_ClearBitplane(PLANEPTR bitplane, int width, int height);
void B_Blit(PLANEPTR destination, int destX, int destY, APTR source, int srcX, int srcY, int sizeW, int sizeH, PLANEPTR oldBackground);
void B_BlitOverwrite(PLANEPTR destination, int destX, int destY, APTR source, int srcX, int srcY, int sizeW, int sizeH);

/*
PLANEPTR B_SaveBackground(PLANEPTR bitplane, int x, int y, int width, int height);
void B_RestoreBackground(PLANEPTR destination, int destX, int destY, PLANEPTR source, int srcX, int srcY, int sizeW, int sizeH);
void B_CheckBobBackground(PLANEPTR bitplane, struct Bob_Sprite *bob);
void B_PlaceBob(PLANEPTR bitplane, struct Bob_Sprite *bob);
*/

void B_BlitTile_ASM(__reg("a0") PLANEPTR destination, __reg("d0") int destX, __reg("d1") int destY, __reg("a1") APTR source, __reg("d2") UWORD tileIndex);
void B_BlitTileRow(PLANEPTR bitplane, UBYTE tileIndices[], PLANEPTR bgTileGraphics, int tilemapRow, int start, int end, int destinationRow);
extern B_BlitTileRow_ASM(__reg("a0") PLANEPTR bitplane, __reg("a1") UBYTE tileIndices[], __reg("a2") PLANEPTR bgTileGraphics, __reg("d0") int row);

#endif
