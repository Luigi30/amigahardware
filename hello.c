#include "hello.h"

#include "colormap.h"
#include "font.h"

UWORD swapEndian(UWORD a) {
	return ((a & 0x00FF) << 8) | ((a & 0xFF00) >> 8); 
}

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

void SetupBitplanes(){
	BPScreen1_Interleaved = AllocMem(80000, MEMF_CHIP|MEMF_CLEAR); //2 screens * 4 bitplanes = 66560 bytes
	BPStatusBar = AllocMem(1024, MEMF_CHIP|MEMF_CLEAR); //until we get the right size for the status bar
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

void S_Initialize(){
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

	SetupBitplanes(); //Allocate the bitplanes and insert them into the copperlist
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
	
	S_Initialize();
	
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
	G_FreeAllGraphics(imageList);
	
	CloseLibrary((struct Library *)GraphicsBase);
}
