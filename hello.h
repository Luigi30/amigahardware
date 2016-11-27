#define true 1
#define false 0

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <exec/memory.h>
#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "copperdefs.h"

#define LOWORD(l) ((WORD)(l))
#define HIWORD(l) ((WORD)(((LONG)(l) >> 16) & 0xFFFF))

PLANEPTR Bitplane1;
PLANEPTR Bitplane2;
PLANEPTR Bitplane3;
PLANEPTR Bitplane4;
PLANEPTR Bitplane5;

void AssumeDirectControl();
void ReleaseSystem();
int WaitForLMB();

extern void WFRAME();
extern void BlitWait();
extern void BlitterClearScreen();

extern UBYTE SPRITE_ship[512];
