#include "hello.h"

#include "colormap.h"
#include "font.h"

struct GfxBase *GraphicsBase;

int spriteX = 160;
int spriteY = 100;

#define SCREEN_TILE_WIDTH  20
#define SCREEN_TILE_HEIGHT 14
#define BITPLANE_TILE_HEIGHT SCREEN_TILE_HEIGHT*2

#define LEVEL_HEIGHT 32

static UBYTE *levelTileMap;

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
	C_BPL1MOD,	0x0000,		/* no modulo */
	C_BPL2MOD,	0x0000,
	
	C_DIWSTRT,	0x2C81,		/* DIWSTRT - top left corner */
	C_DIWSTOP,	0xF4C1,		/* DIWSTOP - bottom right corner */
	C_DDFSTRT,	0x0038,		/* data fetch start ($2981 & $FF /2 - 8.5) */
	C_DDFSTOP,	0x00D0,		/* data fetch stop ($38 + (320 / 2 - 8) */
	
	0xFFFF,	0xFFFE,		/* wait forever */
};

void SetupBitplanes(){

	for(int i=0; i<NUM_BITPLANES; i++){
		//Allocate a bitplane.
		BPScreen1[i] = AllocMem(WIDTH*(HEIGHT*2)/8, MEMF_CHIP|MEMF_CLEAR);	
		BPScreen2[i] = AllocMem(WIDTH*(HEIGHT*2)/8, MEMF_CHIP|MEMF_CLEAR);	
		
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
	//int rowsUntilWrap = LEVEL_HEIGHT - SCREEN_TILE_HEIGHT + 1;
	int screenPointerRowOffset = SCREEN_TILE_HEIGHT;
	
	copperList[17] = (UWORD)0x4200; //enable 4 bitplane display
	
	int scrollingEnabled = TRUE;
	
	while(!done){
		//S_SendString("FrameLoop(): New frame\r\n");
		
		WFRAME(); //we're out of the drawing area
		framecounter++;
		
		//F_PutString(BPScreen1, 31, 220, 40, FONT_8X8, 8, 8, "pos bithc");
		
		if(scrollingEnabled)
		{
			scrollYOffset--; //one pixel per frame
			
			if(scrollYOffset == -16)
			{
				scrollYOffset = 0;
				screenPointerRowOffset--;
				int rowByteOffset = screenPointerRowOffset * 20;
				
				for(int i=0;i<NUM_BITPLANES; i++){
					B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, screenPointerRowOffset);
				}
				
				if(screenPointerRowOffset == 0){
					screenPointerRowOffset = SCREEN_TILE_HEIGHT;
				}
			}
			
			for(int i=0;i<NUM_BITPLANES;i++){
				//Update the graphics pointer for the new frame. 40 bytes per scanline.
				CopperPtrs[i] = BPScreen1[i] + (screenPointerRowOffset * 640) + (scrollYOffset * 40);
			}
		}
		
		WaitBlit();
		
		copperList[1] 	= HIWORD((ULONG)CopperPtrs[0]);
		copperList[3] 	= LOWORD((ULONG)CopperPtrs[0]);
		
		copperList[5] 	= HIWORD((ULONG)CopperPtrs[1]);
		copperList[7] 	= LOWORD((ULONG)CopperPtrs[1]);
		
		copperList[9] 	= HIWORD((ULONG)CopperPtrs[2]);
		copperList[11] 	= LOWORD((ULONG)CopperPtrs[2]);
		
		copperList[13] 	= HIWORD((ULONG)CopperPtrs[3]);
		copperList[15] 	= LOWORD((ULONG)CopperPtrs[3]);
		
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
	
	LoadPalette(egaColorTable, 16);
	
	bgTileGraphics[0] = TILES_bitplane1;
	bgTileGraphics[1] = TILES_bitplane2;
	bgTileGraphics[2] = TILES_bitplane3;
	bgTileGraphics[3] = TILES_bitplane4;

	//The system is disabled, let's do some cool stuff.
	Hardware->cop1lc 	= (ULONG)copperList; //Enable the new copperlist
	Hardware->copjmp1 	= 0; //Strobe to enable copperlist 1
	Hardware->dmacon 	= 0x87C0; //enable DMA
	
	B_ClearBitplane(BPScreen1[0], WIDTH, (HEIGHT*2));
	B_ClearBitplane(BPScreen1[1], WIDTH, (HEIGHT*2));
	B_ClearBitplane(BPScreen1[2], WIDTH, (HEIGHT*2));
	B_ClearBitplane(BPScreen1[3], WIDTH, (HEIGHT*2));
	BlitWait();
	
	//Blit a tile from SPRITE_ship!
	ship = B_AllocateBobSprite();
	ship->position_x = spriteX;
	ship->position_y = spriteY;
	ship->width = 32;
	ship->height = 32;
	ship->bitplanes = 4;
	ship->graphics[0] = SPRITE_ship_bp0;
	ship->graphics[1] = SPRITE_ship_bp1;
	ship->graphics[2] = SPRITE_ship_bp2;
	ship->graphics[3] = SPRITE_ship_bp3;

	Hardware->serper = 371;
	
	//Draw the background tiles.
	//TODO: Why is there one extra tile?
	for(int i=SCREEN_TILE_HEIGHT;i<SCREEN_TILE_HEIGHT*2;i++){
		B_BlitTileRow(BPScreen1, levelTileMap, bgTileGraphics, i);
	}
	
	FrameLoop();
	
	//S_SendString("Post-FrameLoop cleanup\r\n");
	
	//Program is done, time to go now.
	ReleaseSystem();
	
	printf("Exiting\n");
	
	B_FreeBobSprite(ship);
	
	for(int i=0;i<NUM_BITPLANES;i++){
		FreeMem(BPScreen1[i], WIDTH*(HEIGHT*2)/8);
		FreeMem(BPScreen2[i], WIDTH*(HEIGHT*2)/8);
	}
	
	CloseLibrary((struct Library *)GraphicsBase);
}
