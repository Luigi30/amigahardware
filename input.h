#ifndef INPUT_H
#define INPUT_H

#include "globals.h"
#include "amiga.h"
#include "sprites/sprites.h"

#define INP_UP		0x01
#define INP_RIGHT 	0x02
#define INP_DOWN	0x04
#define INP_LEFT	0x08

UBYTE I_CheckJoystick1();
void I_UpdatePlayerPosition(struct SpriteCoordinates *player, UWORD input);

#endif
