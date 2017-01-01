#ifndef GAME_H
#define GAME_H

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>

#include "globals.h"
#include "gfx/blit.h"
#include "gfx/font.h"
#include "game/input.h"
#include "image/loader.h"
#include "sprites/sprites.h"

#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(a,b) (((a)>(b))?(a):(b))

extern struct SpriteCoordinates playerCoordinates;
extern APTR spriteList[];

void GAME_Initialize();
void GAME_UpdateSpriteList(APTR *sprites);
void GAME_Start();
void GAME_FrameLoop();
void GAME_PlayerLeftRiver();
void GAME_CheckTileEffect(UWORD leftBound, UWORD rightBound);

#endif
