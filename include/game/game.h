#ifndef GAME_H
#define GAME_H

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>

#include "globals.h"
#include "blit.h"
#include "input.h"
#include "sprites/sprites.h"
#include "font.h"

#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(a,b) (((a)>(b))?(a):(b))

extern struct SpriteCoordinates playerCoordinates;

void FrameLoop();

#endif
