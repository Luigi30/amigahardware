#define true 1
#define false 0

#define WIDTH 320
#define HEIGHT 232
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

#include "serial.h"
#include "blit.h"
#include "copperdefs.h"

#define LOWORD(l) ((WORD)(l))
#define HIWORD(l) ((WORD)(((LONG)(l) >> 16) & 0xFFFF))

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define NUM_BITPLANES 4

PLANEPTR CopperPtr_Bitplane1;
PLANEPTR CopperPtr_Bitplane2;
PLANEPTR CopperPtr_Bitplane3;
PLANEPTR CopperPtr_Bitplane4;
PLANEPTR CopperPtr_Bitplane5;

//Double buffering!
PLANEPTR BPScreen1[5];
PLANEPTR BPScreen2[5];

PLANEPTR ActiveScreen[5];
PLANEPTR InactiveScreen[5];
int screenBeingDisplayed;

PLANEPTR CopperPtrs[5];
PLANEPTR bgTileGraphics[5];

struct Bob_Sprite *ship;

void AssumeDirectControl();
void ReleaseSystem();
int WaitForLMB();

void B_Blit32x32(PLANEPTR destination, APTR source, int x, int y);

extern int ReadJoystick();
