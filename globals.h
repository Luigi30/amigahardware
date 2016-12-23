#ifndef GLOBALS_H
#define GLOBALS_H

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>

#include "copperdefs.h"

#define true 1
#define false 0

#define LOWORD(l) ((WORD)(l))
#define HIWORD(l) ((WORD)(((LONG)(l) >> 16) & 0xFFFF))

#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(a,b) (((a)>(b))?(a):(b))

/* graphics window */
#define WIDTH 320
#define HEIGHT 208
#define DEPTH 5
#define SCREEN_TILE_WIDTH  20
#define SCREEN_TILE_HEIGHT 15
#define BITPLANE_TILE_HEIGHT SCREEN_TILE_HEIGHT*2
#define NUM_BITPLANES 4

/* sprite */
#define PLAYER_SPRITE_L_BOUND 64  /* first left graphic pixel */
#define PLAYER_SPRITE_R_BOUND 208 /* farthest right we can be */

extern struct ImageData *imageList;

extern PLANEPTR BPScreen1_Interleaved;
extern PLANEPTR BPStatusBar;
extern PLANEPTR CopperPtrs[5];
extern PLANEPTR tilemapBitmap;

extern UBYTE *levelTileMap;

extern int levelHeight;

extern UWORD __chip copperList[];
/* 2 16x32 sprites */
extern UWORD __chip playerSprite1[];
extern UWORD __chip playerSprite2[];

#endif