#include "hello.h"

#include "colormap.h"
#include "font.h"

struct SpriteCoordinates playerCoordinates;

#define SCREEN_TILE_WIDTH  20
#define SCREEN_TILE_HEIGHT 15
#define BITPLANE_TILE_HEIGHT SCREEN_TILE_HEIGHT*2

#define PLAYER_SPRITE_L_BOUND 64  /* first left graphic pixel */
#define PLAYER_SPRITE_R_BOUND 208 /* farthest right we can be */

int levelHeight;
static UBYTE *levelTileMap;

UWORD swapEndian(UWORD a) {
	return ((a & 0x00FF) << 8) | ((a & 0xFF00) >> 8); 
}

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

void P_LoadPalette(UWORD palette[], int entries){
	int i;
	
	for(i=0; i<entries; i++)
		Hardware->color[i] = palette[i];
}

int WaitForLMB(){
	return (PortA->ciapra & 0x40) != 0x40;
}

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

void SetupBitplanes(){
	BPScreen1_Interleaved = AllocMem(80000, MEMF_CHIP|MEMF_CLEAR); //2 screens * 4 bitplanes = 66560 bytes
	BPStatusBar = AllocMem(1024, MEMF_CHIP|MEMF_CLEAR); //until we get the right size for the status bar
}

void F_PutStatusBar(char *str){
	//todo: make variadic function like printf
	
	//pad to 32 characters
	char padded[40];
	strncpy(padded, str, 40);
	padded[39] = 0x00;
	F_PutString(BPStatusBar, 0, 0, FONT_8X8, 8, 8, padded);
	
}

UBYTE S_PlayerTileIndex_Left(struct SpriteCoordinates player, int tileSourceRow){
	/* What tile index is the player's top left pixel on? */
	
	int screenX = player.x - PLAYER_SPRITE_L_BOUND; //get the background X pixel
	int screenY = player.y - 44; //get the background Y pixel
	
	//we have the screen pixel coordinate. now figure out what tile that is
	//todo: this is wrong
	return levelTileMap[(((tileSourceRow-6 + (screenY/8)) * 20) + (screenX/8))];
}

UBYTE S_PlayerTileIndex_Right(struct SpriteCoordinates player, int tileSourceRow){
	/* What tile index is the player's top left pixel on? */
	
	int screenX = player.x + 16 - PLAYER_SPRITE_L_BOUND; //get the background X pixel
	int screenY = player.y - 44; //get the background Y pixel
	
	//we have the screen pixel coordinate. now figure out what tile that is
	//todo: this is wrong
	return levelTileMap[(((tileSourceRow-6 + (screenY/8)) * 20) + (screenX/8))];
}

UBYTE S_TileXOffset(struct SpriteCoordinates player){
	int screenX = player.x - PLAYER_SPRITE_L_BOUND; //get the background X pixel
	return screenX/8;
}

UBYTE S_TileYOffset(struct SpriteCoordinates player){
	int screenY = player.y - 44; //get the background Y pixel
	return screenY/16;
}

void I_UpdatePlayerPosition(UWORD input){
	if((input & INP_DOWN) == INP_DOWN)
		playerCoordinates.y = MIN(200, playerCoordinates.y + 2);
	else if((input & INP_UP) == INP_UP)
		playerCoordinates.y = MAX(0x80, playerCoordinates.y - 2);
	else if((input & INP_LEFT) == INP_LEFT)
		playerCoordinates.x = MAX(PLAYER_SPRITE_L_BOUND, playerCoordinates.x - 1);
	else if((input & INP_RIGHT) == INP_RIGHT)
		playerCoordinates.x = MIN(PLAYER_SPRITE_R_BOUND, playerCoordinates.x + 1);
}

int scrollXOffset = 0;
int scrollYOffset = 0;
void FrameLoop(){
	int done = false;
	
	int framecounter = 0;
	int color = 0;
	int screenPointerRowOffset = SCREEN_TILE_HEIGHT;
	int tileSourceRow = levelHeight - SCREEN_TILE_HEIGHT;
	
	copperList[65] = (UWORD)0x4200; //enable 4 bitplane display

	int scrollingEnabled = TRUE;
	
	while(!done){
		//S_SendString("FrameLoop(): New frame\r\n");
		
		WFRAME(); //we're out of the drawing area
		framecounter++;
		
		/* TODO: encapsulate this */
		I_UpdatePlayerPosition(I_CheckJoystick1());
		
		/* Reset the sprite pointers based on player X and player Y */
		//Y
		playerSprite1[0] = (playerCoordinates.y << 8);
		playerSprite2[0] = (playerCoordinates.y << 8);
		playerSprite1[1] = (playerCoordinates.y+32 << 8);
		playerSprite2[1] = (playerCoordinates.y+32 << 8);
		
		//X
		playerSprite1[0] = (playerSprite1[0] & 0xFF00) | (playerCoordinates.x & 0x00FF);
		playerSprite2[0] = (playerSprite2[0] & 0xFF00) | ((playerCoordinates.x+8) & 0x00FF);
		if(playerCoordinates.x > 256) {
			playerSprite1[1] = playerSprite1[1] | 0x0001;
			playerSprite2[1] = playerSprite2[1] | 0x0001;
		}
		
		/* Update sprites in copperlist */
		copperList[9]	 	= HIWORD((ULONG)playerSprite1);			/* spr 0 */
		copperList[11]		= LOWORD((ULONG)playerSprite1);
		copperList[13]	 	= HIWORD((ULONG)playerSprite2);			/* spr 1 */
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
			//scrollYOffset--; //one pixel per frame
			
			//Blit two tiles per pixel scroll
			switch(scrollYOffset){
				case -1:
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 0, 2, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 0, 2, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -2:
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 2, 4, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 2, 4, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -3:
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 4, 6, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 4, 6, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -4:
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 6, 8, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 6, 8, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -5:
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 8, 10, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 8, 10, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -6:
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 10, 12, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 10, 12, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -7:
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 12, 14, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 12, 14, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -8:
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 14, 16, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 14, 16, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -9:
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 16, 18, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 16, 18, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
					break;
				case -10:
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 18, 20, screenPointerRowOffset);
					B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, tileSourceRow, 18, 20, screenPointerRowOffset+SCREEN_TILE_HEIGHT);
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
						tileSourceRow = levelHeight - SCREEN_TILE_HEIGHT - 1;
					}
					break;
			}
		}
		
		CopperPtrs[0] 	= BPScreen1_Interleaved + ((screenPointerRowOffset+2)*2560) + (scrollYOffset*160);
		CopperPtrs[1] 	= BPScreen1_Interleaved+40 + ((screenPointerRowOffset+2)*2560) + (scrollYOffset*160);
		CopperPtrs[2] 	= BPScreen1_Interleaved+80 + ((screenPointerRowOffset+2)*2560) + (scrollYOffset*160);
		CopperPtrs[3] 	= BPScreen1_Interleaved+120 + ((screenPointerRowOffset+2)*2560) + (scrollYOffset*160);
		
		WaitBlit();
		
		copperList[49] 	= HIWORD((ULONG)CopperPtrs[0]);
		copperList[51] 	= LOWORD((ULONG)CopperPtrs[0]);
		
		copperList[53] 	= HIWORD((ULONG)CopperPtrs[1]);
		copperList[55] 	= LOWORD((ULONG)CopperPtrs[1]);
		
		copperList[57] 	= HIWORD((ULONG)CopperPtrs[2]);
		copperList[59] 	= LOWORD((ULONG)CopperPtrs[2]);
		
		copperList[61] 	= HIWORD((ULONG)CopperPtrs[3]);
		copperList[63] 	= LOWORD((ULONG)CopperPtrs[3]);
		
		copperList[101]	= HIWORD((ULONG)BPStatusBar);
		copperList[103] = LOWORD((ULONG)BPStatusBar);
		
		/* What tile is the player's top-left corner over? */
		UBYTE playerTileIndex = S_PlayerTileIndex_Left(playerCoordinates, tileSourceRow);
		
		
		if(framecounter % 15 == 0){ //.25 second
			char debug[64];
			sprintf(debug, "X$%2x $Y%2x tX%2d tY%2d tIDX$%2x", 
				playerCoordinates.x, 
				playerCoordinates.y,
				S_TileXOffset(playerCoordinates), 
				S_TileYOffset(playerCoordinates),
				S_PlayerTileIndex_Left(playerCoordinates, tileSourceRow));
			F_PutStatusBar(debug);
		}
		
		if(framecounter % 60 == 0){ //1 second
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

int F_LoadTiledLevel(char *filename) {
	printf("F_LoadTiledLevel\r\n");
	//Loads a level into levelTileMap. Returns the level's size in bytes so we can free the buffer later.
	
	BPTR level = Open(filename, MODE_OLDFILE);
	struct FileInfoBlock *fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, TAG_DONE);
	
	ExamineFH(level, fib);
	levelTileMap = AllocMem(fib->fib_Size, MEMF_PUBLIC);
	int bytes = fib->fib_Size;
	
	/* read the header */
	struct TiledBinaryHeader *mapHeader = AllocMem(16, MEMF_PUBLIC|MEMF_CLEAR);
	Read(level, &mapHeader->orientation, 1);
	Read(level, &mapHeader->staggerAxis, 1);
	Read(level, &mapHeader->staggerIndex, 1);
	Read(level, &mapHeader->hexSideLength, 2);
	mapHeader->hexSideLength = swapEndian(mapHeader->hexSideLength);
	Read(level, &mapHeader->mapWidth, 2);
	mapHeader->mapWidth = swapEndian(mapHeader->mapWidth);
	Read(level, &mapHeader->mapHeight, 2);
	mapHeader->mapHeight = swapEndian(mapHeader->mapHeight);
	levelHeight = mapHeader->mapHeight;
	Read(level, &mapHeader->tileWidth, 2);
	mapHeader->tileWidth = swapEndian(mapHeader->tileWidth);
	Read(level, &mapHeader->tileHeight, 2);
	mapHeader->tileHeight = swapEndian(mapHeader->tileHeight);
	Read(level, &mapHeader->tileNumberBits, 1);
	Read(level, &mapHeader->rle, 1);
	Read(level, &mapHeader->layerCount, 1);

	printf("\r\n");
	printf("%s is a %d x %d map\r\n", filename, mapHeader->mapWidth, mapHeader->mapHeight);
	
	int tileNumber = 0;
	for(int row=0;row<mapHeader->mapHeight;row++){
		printf("Reading map row %u of %u\r\n", row, mapHeader->mapHeight);
		for(int column=0;column<mapHeader->mapWidth;column++){
			UWORD tile;
			Read(level, &tile, sizeof(UWORD));
			levelTileMap[tileNumber] = (tile & 0x00FF) - 0x0001;
			tileNumber++;
		}
	}
	
	//Clean up
	FreeDosObject(DOS_FIB, fib);
	Close(level);
	
	return bytes;
}

void G_Initialize(){
	playerCoordinates.x = 96;
	playerCoordinates.y = 200;
}

int main(){	
	GraphicsBase=(struct GfxBase *)OpenLibrary("graphics.library",0);
	
	/* Load graphics files using the OS. */
	G_InitGraphics(&imageList);
	tilemapBitmap = G_FindGraphic(imageList, "BGTILES")->bitmap;
	
	printf("main(): imageList is %p\r\n", imageList);
	printf("main(): bitmap is %p\r\n", imageList->bitmap);
	printf("main(): tilemapBitmap is %p\r\n", tilemapBitmap);
	
	for(int i=0;i<16;i++){
		printf("%02X", imageList->bitmap[i]);
	}
	printf("\r\n");
	
	F_LoadTiledLevel("tilemap.bin");

	SetupBitplanes(); //Allocate the 5 bitplanes and insert them into the copperlist
	AssumeDirectControl(); //Take over the system
	
	Hardware->serper = 371; //9600 bps serial
	
	P_LoadPalette(TILEMAP_palette, 16);

	//The system is disabled, let's do some cool stuff.
	Hardware->cop1lc 	= (ULONG)copperList; //Enable the new copperlist
	Hardware->copjmp1 	= 0; //Strobe to enable copperlist 1
	Hardware->dmacon 	= 0x87E0; //enable DMA
	
	//Sprite debugging
	Hardware->color[17] = 0x0A00;
	Hardware->color[18] = 0x0679;
	Hardware->color[19] = 0x0345;
	
	Hardware->color[21] = 0x0A00;
	Hardware->color[22] = 0x0AAA;
	Hardware->color[23] = 0x0555;

	BlitWait();
	
	G_Initialize();
	
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
	int row = 0;
	for(int i=levelHeight-SCREEN_TILE_HEIGHT;i<levelHeight;i++){
		B_BlitTileRow(BPScreen1_Interleaved, levelTileMap, tilemapBitmap, i, 0, 20, row+SCREEN_TILE_HEIGHT);
		row++;
	}
	
	FrameLoop();
	
	//Program is done, time to go now.
	ReleaseSystem();
	
	printf("Exiting\n");
	
	//B_FreeBobSprite(ship);
	
	FreeMem(BPScreen1_Interleaved, 80000);
	FreeMem(BPStatusBar, 1024); //until we get the right size for the status bar
	//FreeMem(G_FindGraphic("BGTILES")->bitmap, G_FindGraphic("BGTILES")->bytes);
	
	CloseLibrary((struct Library *)GraphicsBase);
}
