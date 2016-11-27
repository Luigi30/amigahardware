#include "hello.h"
#include "colormap.h"
#include "font.h"

#define WIDTH 320
#define HEIGHT 200
#define DEPTH 5

struct GfxBase *GraphicsBase;

/* Begin hardware */
struct Custom *Hardware;
struct CIA *PortA;
struct CIA *PortB;

UWORD SystemInts;
UWORD SystemDMA;

UBYTE *ICRA=(UBYTE *)0xbfed01;
/* End hardware */

struct GfxBase *GraphicsBase;

void AssumeDirectControl(){
	Forbid(); //We have exclusive control.
	
	//Save current interrupts and DMA settings so we can restore them upon exit.
	SystemInts=Hardware->intenar;
	SystemDMA=Hardware->dmaconr;

	//Kill all interrupts and DMA.
	Hardware->intena=0x7fff;
	Hardware->dmacon=0x7fff;

	//Also kill interrupts at the CIA controller itself, which handles keyboard interrupts among others, just to be on the safe side.
	*ICRA=0x7f;
}

void ReleaseSystem(){
	//Kill interrupts and DMA for a moment.
	Hardware->intena=0x7fff;
	Hardware->dmacon=0x7fff;
	
	/*Restore system copper list(s). */
	Hardware->cop1lc=(ULONG)GraphicsBase->copinit;
	Hardware->cop2lc=(ULONG)GraphicsBase->LOFlist;

	/*Restore all interrupts and DMA settings. */
	Hardware->intena=(SystemInts|0xc000);
	Hardware->dmacon=(SystemDMA|0x8100);
	*ICRA=0x9b;
		
	Permit(); //OS is enabled again.
}

void LoadPalette(UWORD palette[], int entries){
	int i;
	
	for(i=0; i<entries; i++)
		Hardware->color[i] = palette[i];
}

int WaitForLMB(){
	return (PortA->ciapra & 0x40) != 0x40;
}

UWORD __chip copperList[] = {	
	C_BPL1PTH, 	0x0000,		/* BPL1PTH - bitplane pointer */
	C_BPL1PTL, 	0x0000,		/* BPL1PTL - bitplane pointer */
	C_BPL2PTH, 	0x0000,		/* BPL2PTH - bitplane pointer */
	C_BPL2PTL, 	0x0000,		/* BPL2PTL - bitplane pointer */
	C_BPL3PTH, 	0x0000,		/* BPL3PTH - bitplane pointer */
	C_BPL3PTL, 	0x0000,		/* BPL3PTL - bitplane pointer */
	C_BPL4PTH, 	0x0000,		/* BPL4PTH - bitplane pointer */
	C_BPL4PTL, 	0x0000,		/* BPL4PTL - bitplane pointer */
	C_BPL5PTH, 	0x0000,		/* BPL5PTH - bitplane pointer */
	C_BPL5PTL, 	0x0000,		/* BPL5PTL - bitplane pointer */	
	C_BPLCON0,	0x7200,		/* 5 bitplanes, enable colorburst */
	C_BPLCON1,	0x0000,		/* no offset */
	C_BPL1MOD,	0x0000,		/* no modulo */
	C_BPL2MOD,	0x0000,
	
	C_DIWSTRT,	0x2C81,		/* DIWSTRT - top left corner */
	C_DIWSTOP,	0xF4C1,		/* DIWSTOP - bottom right corner */
	C_DDFSTRT,	0x0038,		/* data fetch start ($2981 & $FF /2 - 8.5) */
	C_DDFSTOP,	0x00D0,		/* data fetch stop ($38 + (320 / 2 - 8) */
	
	0xFFFF,	0xFFFE,		/* wait forever */
};

void SetupBitplanes(){
	//Allocate bitplanes
	Bitplane1 = AllocMem(WIDTH*HEIGHT/8, MEMF_CHIP);
	Bitplane2 = AllocMem(WIDTH*HEIGHT/8, MEMF_CHIP);
	Bitplane3 = AllocMem(WIDTH*HEIGHT/8, MEMF_CHIP);
	Bitplane4 = AllocMem(WIDTH*HEIGHT/8, MEMF_CHIP);
	Bitplane5 = AllocMem(WIDTH*HEIGHT/8, MEMF_CHIP);
	
	copperList[1] = HIWORD((ULONG)Bitplane1);
	copperList[3] = LOWORD((ULONG)Bitplane1);
	
	copperList[5] = HIWORD((ULONG)Bitplane2);
	copperList[7] = LOWORD((ULONG)Bitplane2);
	
	copperList[9] = HIWORD((ULONG)Bitplane3);
	copperList[11] = LOWORD((ULONG)Bitplane3);
	
	copperList[13] = HIWORD((ULONG)Bitplane4);
	copperList[15] = LOWORD((ULONG)Bitplane4);
	
	copperList[17] = HIWORD((ULONG)Bitplane5);
	copperList[19] = LOWORD((ULONG)Bitplane5);
	
	printf("Bitplane1 is at %p.\n", (APTR)Bitplane1);
	printf("Bitplane2 is at %p.\n", (APTR)Bitplane2);
	printf("Bitplane3 is at %p.\n", (APTR)Bitplane3);
	printf("Bitplane4 is at %p.\n", (APTR)Bitplane4);
	printf("Bitplane5 is at %p.\n", (APTR)Bitplane5);
}

void FrameLoop(){
	int done = false;
	
	int framecounter = 0;
	int color = 0;
	
	while(!done){
		WFRAME();
		framecounter++;
		
		if(framecounter == 60){
			color++;
			F_PutString(color % 32, 40, 8, FONT_8X8, 8, 8, "Assuming direct control...");			
			framecounter = 0;
		}
		
		if(WaitForLMB()){
			done = true;
		}
	}
	
}

void B_ClearBitplane(PLANEPTR bitplane){
	int bltX = 0;
	int bltY = 0;
	int bltOffset = 30 * WIDTH/8 + bltX/8;
	
	int pictureW = WIDTH;
	int pictureH = HEIGHT;
	
	int bltH = pictureH;
	int bltW = pictureW/16;
	UWORD bltSkip = (WIDTH-pictureW)/8;
	
	//Clear a 320x200 rectangle.
	Hardware->dmacon  = 0x8040; //enable blitter DMA
	BlitWait();
	//					0xAMOOB000
	//Hardware->bltcon0 = 0x09F0; //straight copy
	Hardware->bltcon0 = 0x0100; //no source = clear memory
	Hardware->bltcon1 = 0x0000;
	//Hardware->bltapt  = (APTR)FONT_8X8;
	Hardware->bltdpt  = bitplane;
	//Hardware->bltamod = bltSkip;
	Hardware->bltdmod = bltSkip;
	Hardware->bltsize = (UWORD)(bltH*64 + bltW);
}

void B_BlitSpriteShip(int x, int y) {
	UWORD shift = (x % 16) << 12;
	UWORD firstwordmask = 0xFFFF >> shift;
	UWORD lastwordmask = 0xFFFF << (16-shift);
	
	int spriteHeight = 32;
	int spriteWidth = 32;
	
	int bltH = spriteHeight;
	int bltW = spriteWidth/16;
	
	//Increase blit size by 1 word to account for shifts
	UWORD sourceSkip = (64-spriteWidth)/8 - 1;
	UWORD destinationSkip = (WIDTH/8) - bltW*2 - 1;	
	UWORD blitsize = bltH*64 + bltW +1;
	
	Hardware->dmacon 	= 0x8040; //make sure blit DMA is enabled
	BlitWait(); //wait for the blitter to become available
	
	Hardware->bltcon0	= 0x09F0 | shift; //straight copy A to D
	Hardware->bltcon1	= 0x0000 | shift; //shifting the B source
	
	Hardware->bltadat 	= 0x0000; //preload with 0x0000;
	Hardware->bltafwm	= firstwordmask;
	Hardware->bltalwm	= lastwordmask;
	
	Hardware->bltapt	= (APTR)SPRITE_ship;
	Hardware->bltdpt	= (APTR)(Bitplane1 + (y * (WIDTH/16)) + x/8);
	
	Hardware->bltamod	= (UWORD)sourceSkip; //32x32 image plus 1 word = 48x32 output from a 32x32 source
	Hardware->bltdmod	= (UWORD)destinationSkip; //blitting 48x32 onto a 320x200 bitplane
	Hardware->bltsize	= blitsize;
}

void S_SendByte(UBYTE byte){
	Hardware->serdat = 0x0200 | byte;
}

int main(){
	GraphicsBase=(struct GfxBase *)OpenLibrary("graphics.library",0);
	
	//Make the custom hardware structs
	Hardware= (struct Custom *)0xdff000;
	PortA 	= (struct CIA *)0xbfe001;
	PortB 	= (struct CIA *)0xbfe000;
	
	printf("SPRITE_ship is at %p.\n", SPRITE_ship);
	printf("%x\n", (UWORD)(0xFFFF >> 15));
	printf("%x\n", (UWORD)(0xFFFF << 15));
	
	SetupBitplanes(); //Allocate the 5 bitplanes and insert them into the copperlist
	AssumeDirectControl(); //Take over the system
	
	LoadPalette(egaColorTable, 32);

	//The system is disabled, let's do some cool stuff.
	Hardware->cop1lc 	= (ULONG)copperList; //Enable the new copperlist
	Hardware->copjmp1 	= 0; //Strobe to enable copperlist 1
	Hardware->dmacon 	= 0x87C0; //enable DMA
	Hardware->color[1]	= 0xFFF; //override color1 to white since we have 1bpp graphics
	
	B_ClearBitplane(Bitplane1);
	B_ClearBitplane(Bitplane2);
	B_ClearBitplane(Bitplane3);
	B_ClearBitplane(Bitplane4);
	B_ClearBitplane(Bitplane5);
	BlitWait();
	
	//Blit a tile from SPRITE_ship!
	B_BlitSpriteShip(0, 0);
	B_BlitSpriteShip(8, 32);
	B_BlitSpriteShip(16, 64);
	B_BlitSpriteShip(24, 96);
	
	FrameLoop();
	
	//Program is done, time to go now.
	ReleaseSystem();
	
	printf("Exiting\n");
	
	FreeMem(Bitplane1, WIDTH*HEIGHT/8);
	FreeMem(Bitplane2, WIDTH*HEIGHT/8);
	FreeMem(Bitplane3, WIDTH*HEIGHT/8);
	FreeMem(Bitplane4, WIDTH*HEIGHT/8);
	FreeMem(Bitplane5, WIDTH*HEIGHT/8);	
	CloseLibrary((struct Library *)GraphicsBase);
}
