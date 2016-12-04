#include "hello.h"

#include "colormap.h"
#include "font.h"

struct GfxBase *GraphicsBase;

int spriteX = 160;
int spriteY = 100;

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

void I_CheckJoystick(struct Bob_Sprite *bob){
	//check joystick
	UWORD input = Hardware->joy1dat; //right port is joystick
	if(((input & 0x0002) == 0x0002) ^ ((input & 0x0001) == 0x0001))
	{
		bob->position_y++;
	}
	else if(((input & 0x0200) == 0x0200) ^ ((input & 0x0100) == 0x0100))
	{
		bob->position_y--;
	}
	else if(input & 0x0002)
	{
		bob->position_x++;
	}
	else if(input & 0x0200)
	{
		bob->position_x--;
	}
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
	C_BPLCON0,	0x0200,		/* no bitplanes, enable colorburst */
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
	BPScreen1[0] = AllocMem(WIDTH*HEIGHT/8, MEMF_CHIP);
	BPScreen1[1] = AllocMem(WIDTH*HEIGHT/8, MEMF_CHIP);
	BPScreen1[2] = AllocMem(WIDTH*HEIGHT/8, MEMF_CHIP);
	BPScreen1[3] = AllocMem(WIDTH*HEIGHT/8, MEMF_CHIP);
	BPScreen1[4] = AllocMem(WIDTH*HEIGHT/8, MEMF_CHIP);
	
	CopperPtr_Bitplane1 = BPScreen1[0];
	CopperPtr_Bitplane2 = BPScreen1[1];
	CopperPtr_Bitplane3 = BPScreen1[2];
	CopperPtr_Bitplane4 = BPScreen1[3];
	CopperPtr_Bitplane5 = BPScreen1[4];
	
	printf("Screen1_Bitplane0 is at %p.\n", (APTR)BPScreen1[0]);
	printf("Screen1_Bitplane1 is at %p.\n", (APTR)BPScreen1[1]);
	printf("Screen1_Bitplane2 is at %p.\n", (APTR)BPScreen1[2]);
	printf("Screen1_Bitplane3 is at %p.\n", (APTR)BPScreen1[3]);
	printf("Screen1_Bitplane4 is at %p.\n", (APTR)BPScreen1[4]);
}

int scrollXOffset = 0;
int scrollYOffset = 0;
void FrameLoop(){
	int done = false;
	
	int framecounter = 0;
	int color = 0;
	
	copperList[21] = (UWORD)0x7200; //enable the 5 bitplane display
	
	while(!done){
		//S_SendString("FrameLoop(): New frame\r\n");
		
		WFRAME(); //we're out of the drawing area
		framecounter++;
		
		//scrollYOffset++;
		//if(scrollYOffset == 16){
		//	scrollYOffset = 0;
		//}
		
		CopperPtr_Bitplane1 = BPScreen1[0] + (scrollYOffset * 40);
		CopperPtr_Bitplane2 = BPScreen1[1] + (scrollYOffset * 40);
		CopperPtr_Bitplane3 = BPScreen1[2] + (scrollYOffset * 40);
		CopperPtr_Bitplane4 = BPScreen1[3] + (scrollYOffset * 40);
		CopperPtr_Bitplane5 = BPScreen1[4] + (scrollYOffset * 40);
		
		//Update the copper bitplane pointers
		copperList[1] 	= HIWORD((ULONG)CopperPtr_Bitplane1);
		copperList[3] 	= LOWORD((ULONG)CopperPtr_Bitplane1);
		
		copperList[5] 	= HIWORD((ULONG)CopperPtr_Bitplane2);
		copperList[7] 	= LOWORD((ULONG)CopperPtr_Bitplane2);
		
		copperList[9] 	= HIWORD((ULONG)CopperPtr_Bitplane3);
		copperList[11] 	= LOWORD((ULONG)CopperPtr_Bitplane3);
		
		copperList[13] 	= HIWORD((ULONG)CopperPtr_Bitplane4);
		copperList[15] 	= LOWORD((ULONG)CopperPtr_Bitplane4);
		
		copperList[17] 	= HIWORD((ULONG)CopperPtr_Bitplane5);
		copperList[19] 	= LOWORD((ULONG)CopperPtr_Bitplane5);
		
		B_PlaceBob(BPScreen1, ship);
		
		I_CheckJoystick(ship); //update ship's position based on joystick
		
		if(framecounter == 60){
			//F_PutString(BPScreen1, color % 32, 40, 8, FONT_8X8, 8, 8, "Assuming direct control...");
			//color++;			
			framecounter = 0;
		}
		
		if(WaitForLMB()){
			done = true;
		}
		
	}
	
}

int main(){	
	GraphicsBase=(struct GfxBase *)OpenLibrary("graphics.library",0);
	
	//printf("SPRITE_ship is at %p.\n", SPRITE_ship);
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
	
	B_ClearBitplane(BPScreen1[0], WIDTH, HEIGHT);
	B_ClearBitplane(BPScreen1[1], WIDTH, HEIGHT);
	B_ClearBitplane(BPScreen1[2], WIDTH, HEIGHT);
	B_ClearBitplane(BPScreen1[3], WIDTH, HEIGHT);
	B_ClearBitplane(BPScreen1[4], WIDTH, HEIGHT);
	BlitWait();
	
	//Blit a tile from SPRITE_ship!
	ship = B_AllocateBobSprite();
	ship->position_x = spriteX;
	ship->position_y = spriteY;
	ship->width = 32;
	ship->height = 32;
	ship->bitplanes = 5;
	ship->graphics[0] = SPRITE_ship;
	ship->graphics[1] = SPRITE_ship;
	ship->graphics[2] = SPRITE_ship;
	ship->graphics[3] = SPRITE_ship;
	ship->graphics[4] = SPRITE_ship;
	//B_PlaceBob(BPScreen1, ship);
	
	//Create the tiles in the background.
	for(int y=0;y<14;y++){
		for(int x=0;x<20;x++){
			B_Blit(BPScreen1[0], x*16, y*16, (APTR)TILES_bitplane1, 0, 0, 16, 16, NULL);
			B_Blit(BPScreen1[1], x*16, y*16, (APTR)TILES_bitplane2, 0, 0, 16, 16, NULL);
			B_Blit(BPScreen1[2], x*16, y*16, (APTR)TILES_bitplane3, 0, 0, 16, 16, NULL);
			B_Blit(BPScreen1[3], x*16, y*16, (APTR)TILES_bitplane4, 0, 0, 16, 16, NULL);
		}		
	}
	
	Hardware->serper = 371;
	//S_SendString("Testing string\r\n");
	
	FrameLoop();
	
	//S_SendString("Post-FrameLoop cleanup\r\n");
	
	//Program is done, time to go now.
	ReleaseSystem();
	
	printf("Exiting\n");
	
	B_FreeBobSprite(ship);
	
	FreeMem(BPScreen1[0], WIDTH*HEIGHT/8);
	FreeMem(BPScreen1[1], WIDTH*HEIGHT/8);
	FreeMem(BPScreen1[2], WIDTH*HEIGHT/8);
	FreeMem(BPScreen1[3], WIDTH*HEIGHT/8);
	FreeMem(BPScreen1[4], WIDTH*HEIGHT/8);		
	
	CloseLibrary((struct Library *)GraphicsBase);
}
