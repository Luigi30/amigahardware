#include "globals.h"

struct ImageData *imageList;

PLANEPTR BPScreen1_Interleaved;
PLANEPTR BPStatusBar;
PLANEPTR CopperPtrs[5];
PLANEPTR tilemapBitmap;

UBYTE *levelTileMap;

int levelHeight;

UWORD __chip copperList[] = {	
	/* Define a 320x208 screen */
	C_DIWSTRT,	0x1F81,		/* @0 DIWSTRT - top left corner */
	C_DIWSTOP,	0xEFC1,		/* @2 DIWSTOP - bottom right corner */
	C_DDFSTRT,	0x0038,		/* @4 data fetch start ($2981 & $FF /2 - 8.5) */
	C_DDFSTOP,	0x00D0,		/* @6 data fetch stop ($38 + (320 / 2 - 8) */
	
	0x120,		0x0000,		/* @8 SPR0PTH */
	0x122,		0x0000,		/* @10 SPR0PTL */
	0x124,		0x0000,		/* @12 SPR1PTH */
	0x126,		0x0000,		/* @14 SPR1PTL */
	0x128,		0x0000,		/* @16 SPR2PTH */
	0x12A,		0x0000,		/* @18 SPR2PTL */
	0x12C,		0x0000,		/* @20 SPR3PTH */
	0x12E,		0x0000,		/* @22 SPR3PTL */
	0x130,		0x0000,		/* @24 SPR4PTH */
	0x132,		0x0000,		/* @26 SPR4PTL */
	0x134,		0x0000,		/* @28 SPR5PTH */
	0x136,		0x0000,		/* @30 SPR5PTL */
	0x138,		0x0000,		/* @32 SPR6PTH */
	0x13A,		0x0000,		/* @34 SPR6PTL */
	0x13C,		0x0000,		/* @36 SPR7PTH */
	0x13E,		0x0000,		/* @38 SPR7PTL */
	
	/* top border */
	/* HBLANK conventional positions are 07 and DF */
	0x1E07,		0xFF00,		/* @40 wait for line $2B */
	0x180,		0x0FFF,		/* @42 change the background color to white for one scanline */
	0x1F07,		0xFF00,		/* @44 back to black */
	0x180,		0x0000,		/* @46
	
	/* set bitplane pointers */
	C_BPL1PTH, 	0x0000,		/* @48 BPL1PTH - bitplane pointer */
	C_BPL1PTL, 	0x0000,		/* @50 BPL1PTL - bitplane pointer */
	C_BPL2PTH, 	0x0000,		/* @52 BPL2PTH - bitplane pointer */
	C_BPL2PTL, 	0x0000,		/* @54 BPL2PTL - bitplane pointer */
	C_BPL3PTH, 	0x0000,		/* @56 BPL3PTH - bitplane pointer */
	C_BPL3PTL, 	0x0000,		/* @58 BPL3PTL - bitplane pointer */
	C_BPL4PTH, 	0x0000,		/* @60 BPL4PTH - bitplane pointer */
	C_BPL4PTL, 	0x0000,		/* @62 BPL4PTL - bitplane pointer */
	
	C_BPLCON0,	0x0200,		/* @64 no bitplanes, enable colorburst */
	C_BPLCON1,	0x0000,		/* @66 no offset */
	0x104,		0x0024,		/* @68 sprites > playfields */
	0x108,		120,		/* @70 bpl1mod interleaved */
	0x10A,		120,		/* @72 bpl2mod interleaved */
	
	/* bottom border */
	0xEF07,		0xFF00,		/* @74 wait for line */
	0x180,		0x0A00,		/* @76 change the background color to red for one scanline */
	
	C_BPLCON0,	0x0200,		/* @78 no bitplanes, enable colorburst */
	C_BPLCON1,	0x0000,		/* @80 no offset */
	0x104,		0x0024,		/* @82 sprites > playfields */
	0x108,		0x0000,		/* @84 bpl1mod not interleaved */
	0x10A,		0x0000,		/* @86 bpl2mod not interleaved */	
	
	0xF007,		0xFF00,		/* @88 back to black */
	0x180,		0x0000,		/* @90 */
	
	/* bottom display window */	
	C_DIWSTRT,	0xF181,		/* @92 DIWSTRT - top left corner */
	C_DIWSTOP,	0xFAC1,		/* @94 DIWSTOP - bottom right corner */
	C_DDFSTRT,	0x0038,		/* @96 data fetch start ($2981 & $FF /2 - 8.5) */
	C_DDFSTOP,	0x00D0,		/* @98 data fetch stop ($38 + (320 / 2 - 8) */
	C_BPL1PTH, 	0x0000,		/* @100 BPL1PTH - bitplane pointer */
	C_BPL1PTL, 	0x0000,		/* @102 BPL1PTL - bitplane pointer */
	C_BPLCON0,	0x1200,		/* @104 one bitplane, enable colorburst */
	C_BPLCON1,	0x0000,		/* @106 no offset */
	0x104,		0x0024,		/* @108 sprites > playfields */
	0x108,		0x0000,		/* @110 bpl1mod not interleaved */
	0x10A,		0x0000,		/* @112 bpl2mod not interleaved */	
	0x180,		0x0AAA,		/* @114 text color */
	
	0xF007,		0xFF00,		/* @116 */
	0x180,		0x0004,		/* @118 */
	
	0xFB07,		0xFF00,		/* @120 */
	0x180,		0x0000,		/* @122 */

	0xFFFF,	0xFFFE,		/* wait forever */
};

/* 2 16x32 sprites */
UWORD __chip playerSprite1[] =
{
	0xD060,0xF000,
	0x0000,0x0000,
	0x0000,0x0001,
	0x0000,0x0001,
	0x0000,0x0001,
	0x0002,0x0003,
	0x0000,0x0003,
	0x0000,0x0003,
	0x0000,0x0003,
	0x0004,0x0007,
	0x0c00,0x0007,
	0x0c01,0x0c07,
	0x0001,0x0c07,
	0x0001,0x0c07,
	0x0001,0x1e07,
	0x0001,0x1e07,
	0x0001,0x1e07,
	0x0010,0x1e1f,
	0x0000,0x1e3f,
	0x01c0,0x1fff,
	0x1fc0,0x1fff,
	0x1fc0,0x1fff,
	0x1fc0,0x1fff,
	0x1fc0,0x1fff,
	0x1fc0,0x1fff,
	0x0fc0,0x0fff,
	0x07c0,0x07ff,
	0x0000,0x003f,
	0x0000,0x003e,
	0x0038,0x0000,
	0x0038,0x0000,
	0x0010,0x0000,
	0x0000,0x0000,
	0x0000,0x0000,
};

UWORD __chip playerSprite2[] =
{	
	0xD068,0xF000,
	0x0000,0x0000,
	0x0000,0x8000,
	0x0000,0x8000,
	0x0000,0x8000,
	0x4000,0xc000,
	0x0000,0xc000,
	0x0000,0xc000,
	0x0000,0xc000,
	0x2000,0xe000,
	0x0030,0xe000,
	0x8030,0xe030,
	0x8000,0xe030,
	0x8000,0xe030,
	0x8000,0xe078,
	0x8000,0xe078,
	0x8000,0xe078,
	0x0800,0xf878,
	0x0000,0xfc78,
	0x0380,0xfff8,
	0x03f8,0xfff8,
	0x03f8,0xfff8,
	0x03f8,0xfff8,
	0x03f8,0xfff8,
	0x03f8,0xfff8,
	0x03f0,0xfff0,
	0x03e0,0xffe0,
	0x0000,0xfc00,
	0x0000,0x7c00,
	0x1c00,0x0000,
	0x1c00,0x0000,
	0x0800,0x0000,
	0x0000,0x0000,
};