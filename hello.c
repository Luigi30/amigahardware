#include "hello.h"

#include "colormap.h"
#include "font.h"

struct GfxBase *GraphicsBase;

int spriteX = 160;
int spriteY = 100;

#define SCREEN_TILE_WIDTH  20
#define SCREEN_TILE_HEIGHT 13
#define BITPLANE_TILE_HEIGHT SCREEN_TILE_HEIGHT*2

#define LEVEL_HEIGHT 26

static UBYTE *levelTileMap;

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
		bob->position_y = bob->position_y + 2;
	}
	else if(((input & 0x0200) == 0x0200) ^ ((input & 0x0100) == 0x0100))
	{
		bob->position_y = bob->position_y - 2;
	}
	else if(input & 0x0002)
	{
		bob->position_x = bob->position_x + 2;
	}
	else if(input & 0x0200)
	{
		bob->position_x = bob->position_x - 2;
	}
}

int WaitForLMB(){
	return (PortA->ciapra & 0x40) != 0x40;
}

UWORD __chip copperList[] = {	
	/* Define a 320x208 screen */
	C_DIWSTRT,	0x2C81,		/* DIWSTRT - top left corner */
	C_DIWSTOP,	0xFCC1,		/* DIWSTOP - bottom right corner */
	C_DDFSTRT,	0x0038,		/* data fetch start ($2981 & $FF /2 - 8.5) */
	C_DDFSTOP,	0x00D0,		/* data fetch stop ($38 + (320 / 2 - 8) */
	
	0x120,		0x0000,		/* SPR0PTH */
	0x122,		0x0000,		/* SPR0PTL */
	0x124,		0x0000,		/* SPR1PTH */
	0x126,		0x0000,		/* SPR1PTL */
	0x128,		0x0000,		/* SPR2PTH */
	0x12A,		0x0000,		/* SPR2PTL */
	0x12C,		0x0000,		/* SPR3PTH */
	0x12E,		0x0000,		/* SPR3PTL */
	0x130,		0x0000,		/* SPR4PTH */
	0x132,		0x0000,		/* SPR4PTL */
	0x134,		0x0000,		/* SPR5PTH */
	0x136,		0x0000,		/* SPR5PTL */
	0x138,		0x0000,		/* SPR6PTH */
	0x13A,		0x0000,		/* SPR6PTL */
	0x13C,		0x0000,		/* SPR7PTH */
	0x13E,		0x0000,		/* SPR7PTL */
	
	/* debugging: top border */
	/* HBLANK conventional positions are 07 and DF */
	0x2B07,	0xFF00,		/* wait for line $2B */
	0x180,	0x0FFF,		/* change the background color to white for one scanline */
	0x2C07,	0xFF00,		/* back to black */
	0x180,	0x0000,
	
	/* set bitplane pointers */
	C_BPL1PTH, 	0x0000,		/* BPL1PTH - bitplane pointer */
	C_BPL1PTL, 	0x0000,		/* BPL1PTL - bitplane pointer */
	C_BPL2PTH, 	0x0000,		/* BPL2PTH - bitplane pointer */
	C_BPL2PTL, 	0x0000,		/* BPL2PTL - bitplane pointer */
	C_BPL3PTH, 	0x0000,		/* BPL3PTH - bitplane pointer */
	C_BPL3PTL, 	0x0000,		/* BPL3PTL - bitplane pointer */
	C_BPL4PTH, 	0x0000,		/* BPL4PTH - bitplane pointer */
	C_BPL4PTL, 	0x0000,		/* BPL4PTL - bitplane pointer */
	
	C_BPLCON0,	0x0200,		/* no bitplanes, enable colorburst */
	C_BPLCON1,	0x0000,		/* no offset */
	0x104,		0x0024,		/* sprites > playfields */
	C_BPL1MOD,	0x0000,		/* no modulos */
	C_BPL2MOD,	0x0000,
	
	/* debugging: bottom border */
	0xFC07,	0xFF00,		/* wait for line $F4 */
	0x180,	0x0FFF,		/* change the background color to white for one scanline */
	0xFD07,	0xFF00,		/* back to black */
	0x180,	0x0000,	
	
	0xFFFF,	0xFFFE,		/* wait forever */
};

void SetupBitplanes(){

	for(int i=0; i<NUM_BITPLANES; i++){
		//Allocate a bitplane.
		BPScreen1[i] = AllocMem(WIDTH*(2+HEIGHT*2)/8, MEMF_CHIP|MEMF_CLEAR);
		
		//Position the copper pointer at the bottom screen (14 rows) of our bitplane.
		CopperPtrs[i] = BPScreen1[i] + ((WIDTH/8) * (BITPLANE_TILE_HEIGHT/2));
	}
}

int scrollXOffset = 0;
int scrollYOffset = 0;
void FrameLoop(){
	int done = false;
	
	int framecounter = 0;
	int color = 0;
	int screenPointerRowOffset = SCREEN_TILE_HEIGHT;
	int tileSourceRow = SCREEN_TILE_HEIGHT;
	
	copperList[65] = (UWORD)0x4200; //enable 4 bitplane display

	int scrollingEnabled = TRUE;
	
	while(!done){
		//S_SendString("FrameLoop(): New frame\r\n");
		
		WFRAME(); //we're out of the drawing area
		framecounter++;
		
		/* Reset the sprite pointers */
		copperList[9]	 	= HIWORD((ULONG)playerSprite1);		/* spr 0 */
		copperList[11]		= LOWORD((ULONG)playerSprite1);
		copperList[13]	 	= HIWORD((ULONG)playerSprite2);	/* spr 1 */
		copperList[15]		= LOWORD((ULONG)playerSprite2);
		copperList[17]	 	= HIWORD((ULONG)SPRITE_placeholder);	/* spr 2 */
		copperList[19]		= LOWORD((ULONG)SPRITE_placeholder);
		copperList[21]	 	= HIWORD((ULONG)SPRITE_placeholder);	/* spr 3 */
		copperList[23]		= LOWORD((ULONG)SPRITE_placeholder);
		copperList[25]	 	= HIWORD((ULONG)SPRITE_placeholder);	/* spr 4 */
		copperList[27]		= LOWORD((ULONG)SPRITE_placeholder);
		copperList[29]	 	= HIWORD((ULONG)SPRITE_placeholder);	/* spr 5 */
		copperList[31]		= LOWORD((ULONG)SPRITE_placeholder);
		copperList[33]	 	= HIWORD((ULONG)SPRITE_placeholder);	/* spr 6 */
		copperList[35]		= LOWORD((ULONG)SPRITE_placeholder);
		copperList[37]	 	= HIWORD((ULONG)SPRITE_placeholder);	/* spr 7 */
		copperList[39]		= LOWORD((ULONG)SPRITE_placeholder);
		
		//F_PutString(BPScreen1, 31, 220, 40, FONT_8X8, 8, 8, "pos bithc");
		
		if(scrollingEnabled)
		{
			scrollYOffset--; //one pixel per frame
			
			//Blit two tiles per pixel scroll
			switch(scrollYOffset){
				case -1:
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 0, 2, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 0, 2, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -2:
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 2, 4, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 2, 4, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -3:
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 4, 6, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 4, 6, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -4:
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 6, 8, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 6, 8, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -5:
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 8, 10, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 8, 10, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -6:
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 10, 12, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 10, 12, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -7:
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 12, 14, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 12, 14, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -8:
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 14, 16, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 14, 16, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -9:
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 16, 18, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 16, 18, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -10:
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 18, 20, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, tileSourceRow, 18, 20, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -16:
					//End of a row.
					scrollYOffset = 0;
					screenPointerRowOffset--;
					tileSourceRow--;
					if(screenPointerRowOffset == -1){
						screenPointerRowOffset = SCREEN_TILE_HEIGHT-1;
					}
					if(tileSourceRow == -1){
						tileSourceRow = SCREEN_TILE_HEIGHT-1;
					}
					break;
			}
		}
		
		for(int i=0;i<NUM_BITPLANES;i++){
			//Update the graphics pointer for the new frame. 40 bytes per scanline.
			CopperPtrs[i] = BPScreen1[i] + ((screenPointerRowOffset+1) * 640) + (scrollYOffset * 40);
		}
		
		WaitBlit();
		
		copperList[49] 	= HIWORD((ULONG)CopperPtrs[0]);
		copperList[51] 	= LOWORD((ULONG)CopperPtrs[0]);
		
		copperList[53] 	= HIWORD((ULONG)CopperPtrs[1]);
		copperList[55] 	= LOWORD((ULONG)CopperPtrs[1]);
		
		copperList[57] 	= HIWORD((ULONG)CopperPtrs[2]);
		copperList[59] 	= LOWORD((ULONG)CopperPtrs[2]);
		
		copperList[61] 	= HIWORD((ULONG)CopperPtrs[3]);
		copperList[63] 	= LOWORD((ULONG)CopperPtrs[3]);
		
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

void early_abort(char *msg){
	printf("Early abort.\n");
	printf(msg);
	exit(RETURN_FAIL);
}

int F_LoadLevel(char *filename) {
	//Loads a level into levelTileMap. Returns the level's size in bytes so we can free the buffer later.
	
	BPTR level = Open(filename, MODE_OLDFILE);
	struct FileInfoBlock *fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, TAG_DONE);
	
	ExamineFH(level, fib);
	levelTileMap = AllocMem(fib->fib_Size, MEMF_PUBLIC);
	int bytesRead = Read(level, levelTileMap, fib->fib_Size);
	
	//Clean up
	FreeDosObject(DOS_FIB, fib);
	Close(level);
	
	return bytesRead;
}

int main(){	
	GraphicsBase=(struct GfxBase *)OpenLibrary("graphics.library",0);
	
	//Use the OS to load some files.
	int levelSize = F_LoadLevel("level.dat");
	if(levelSize == -1){
		early_abort("Failed to load level!\n");
	}
	
	printf("Level size is %d bytes\n", levelSize);
	
	if(levelSize % 20 != 0){
		early_abort("Level length is invalid. Must be a multiple of 20 bytes.");
	}

	SetupBitplanes(); //Allocate the 5 bitplanes and insert them into the copperlist
	AssumeDirectControl(); //Take over the system
	
	Hardware->serper = 371; //9600 bps serial
	
	LoadPalette(egaColorTable, 16);
	
	bgTileGraphics[0] = TILES_bitplane1;
	bgTileGraphics[1] = TILES_bitplane2;
	bgTileGraphics[2] = TILES_bitplane3;
	bgTileGraphics[3] = TILES_bitplane4;

	//The system is disabled, let's do some cool stuff.
	Hardware->cop1lc 	= (ULONG)copperList; //Enable the new copperlist
	Hardware->copjmp1 	= 0; //Strobe to enable copperlist 1
	Hardware->dmacon 	= 0x87E0; //enable DMA
	
	//Sprite debugging
	Hardware->color[17] = 0x0A00;
	Hardware->color[18] = 0x0AAA;
	Hardware->color[19] = 0x0555;
	
	Hardware->color[21] = 0x0A00;
	Hardware->color[22] = 0x0AAA;
	Hardware->color[23] = 0x0555;
	
	for(int i=0;i<NUM_BITPLANES;i++){
		B_ClearBitplane(BPScreen1[i], WIDTH, (HEIGHT*2));
	}

	BlitWait();
	
	//Blit a tile from SPRITE_ship!
	ship = B_AllocateBobSprite();
	/*
	ship->position_x = spriteX;
	ship->position_y = spriteY;
	ship->width = 32;
	ship->height = 32;
	ship->bitplanes = 4;
	ship->graphics[0] = SPRITE_ship_bp0;
	ship->graphics[1] = SPRITE_ship_bp1;
	ship->graphics[2] = SPRITE_ship_bp2;
	ship->graphics[3] = SPRITE_ship_bp3;
	ship->mask = SPRITE_ship_mask;
	for(int i=0;i<512;i++){
		ship->mask[i] = ship->graphics[0][i] | ship->graphics[1][i] | ship->graphics[2][i] | ship->graphics[3][i];
	}
	*/
	
	//Draw the background tiles.
	for(int i=SCREEN_TILE_HEIGHT;i<SCREEN_TILE_HEIGHT*2;i++){
		B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, i, 0, 20, i);
	}
	
	FrameLoop();
	
	//S_SendString("Post-FrameLoop cleanup\r\n");
	
	//Program is done, time to go now.
	ReleaseSystem();
	
	printf("Exiting\n");
	
	//B_FreeBobSprite(ship);
	
	printf("Freeing bitplanes\r\n");
	for(int i=0;i<NUM_BITPLANES;i++){
		FreeMem(BPScreen1[i], WIDTH*(2+HEIGHT*2)/8);
	}
	printf("Bitplanes freed\r\n");
	
	CloseLibrary((struct Library *)GraphicsBase);
}
