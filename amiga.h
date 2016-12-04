#ifndef AMIGA_H
#define AMIGA_H

#include <hardware/cia.h>
#include <hardware/custom.h>

extern struct Custom *Hardware;
extern struct CIA *PortA;
extern struct CIA *PortB;
extern UWORD SystemInts;
extern UWORD SystemDMA;
extern UBYTE *ICRA;

extern void WFRAME();
extern void BlitWait();
extern void BlitterClearScreen();

extern UBYTE SPRITE_ship[512];
extern UBYTE TILES_bitplane1[512];
extern UBYTE TILES_bitplane2[512];
extern UBYTE TILES_bitplane3[512];
extern UBYTE TILES_bitplane4[512];

#endif
