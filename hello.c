#include "hello.h"
#include "colormap.h"
#include "font.h"
#include "copperdefs.h"

#define WIDTH 320
#define HEIGHT 200
#define DEPTH 1

struct GfxBase *GraphicsBase;

UBYTE *bitplane1;
UWORD NULLSPRITE[] = { 0, 0, 0, 0 };

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
	C_BPLCON0,	0x1200,		/* one bitplane, enable colorburst */
	C_BPLCON1,	0x0000,		/* no offset */
	C_BPL1MOD,	0x0000,		/* no modulo */
	
	C_DIWSTRT,	0x2C81,		/* DIWSTRT - top left corner */
	C_DIWSTOP,	0xF4C1,		/* DIWSTOP - bottom right corner */
	C_DDFSTRT,	0x0038,		/* data fetch start ($2981 & $FF /2 - 8.5) */
	C_DDFSTOP,	0x00D0,		/* data fetch stop ($38 + (320 / 2 - 8) */
	
	/* sprite placeholders */
	C_SPR0PTH,	0x0000,
	C_SPR0PTL,	0x0000,
	C_SPR1PTH,	0x0000,
	C_SPR1PTL,	0x0000,
	C_SPR2PTH,	0x0000,
	C_SPR2PTL,	0x0000,
	C_SPR3PTH,	0x0000,
	C_SPR3PTL,	0x0000,
	C_SPR4PTH,	0x0000,
	C_SPR4PTL,	0x0000,
	C_SPR5PTH,	0x0000,
	C_SPR5PTL,	0x0000,
	C_SPR6PTH,	0x0000,
	C_SPR6PTL,	0x0000,
	C_SPR7PTH,	0x0000,
	C_SPR7PTL,	0x0000,
	
	0xFFFF,	0xFFFE,		/* wait forever */
};

int main(){
	int i;
	
	Hardware=(struct Custom *)0xdff000;
	PortA = (struct CIA *)0xbfe001;
	PortB = (struct CIA *)0xbfe000;
	
	GraphicsBase=(struct GfxBase *)OpenLibrary("graphics.library",0);
	
	//Allocate a bitplane
	bitplane1 = AllocMem(WIDTH*HEIGHT/8, MEMF_CHIP);
	
	printf("bitplane1 is %p\n", bitplane1);
	printf("copperlist address is %p\n", (ULONG)copperList);
	
	copperList[1] = HIWORD((ULONG)bitplane1);
	copperList[3] = LOWORD((ULONG)bitplane1);
	
	AssumeDirectControl();
	LoadPalette(egaColorTable, 32);

	//The system is disabled, let's do some cool stuff.
	for(i=0;i<WIDTH*HEIGHT/8;i++){
		bitplane1[i] = 0x00;
	}

	F_PutString(bitplane1, 40, 48, FONT_8X8, 8, 8, "Assuming direct control...");
	
	Hardware->cop1lc = (ULONG)copperList; //Enable the new copperlist
	Hardware->copjmp1 = 0; //Strobe to enable copperlist 1
	Hardware->dmacon = 0x83C0; //enable DMA
	Hardware->color[1] = 0xFFF; //override color1 to white since we have 1bpp graphics
	
	/* Do some cool stuff! */
	
	while(WaitForLMB()){}; 	//wait for LMB to be up...
	while(!WaitForLMB()){}; //...then wait for a click

	ReleaseSystem();
	FreeMem(bitplane1, WIDTH*HEIGHT/8);
	printf("Exiting\n");
	
	CloseLibrary((struct Library *)GraphicsBase);
}
