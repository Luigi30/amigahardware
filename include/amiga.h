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

extern UBYTE SPRITE_ship_bp0[];
extern UBYTE SPRITE_ship_bp1[];
extern UBYTE SPRITE_ship_bp2[];
extern UBYTE SPRITE_ship_bp3[];
extern UBYTE SPRITE_ship_mask[];

extern UBYTE SPRITE_player[];

extern UBYTE TILES_bitplane1[];
extern UBYTE TILES_bitplane2[];
extern UBYTE TILES_bitplane3[];
extern UBYTE TILES_bitplane4[];

//extern UBYTE TILEMAP[];
extern UWORD TILEMAP_palette[];

static int WaitForLMB(){
	return (PortA->ciapra & 0x40) != 0x40;
}

#endif
