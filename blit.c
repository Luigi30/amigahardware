#include "blit.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(a,b) (((a)>(b))?(a):(b))

void B_ClearBitplane(PLANEPTR bitplane, int width, int height){
	int bltX = 0;
	int bltY = 0;
	int bltOffset = 30 * width/8 + bltX/8;
	
	int pictureW = width;
	int pictureH = height;
	
	int bltH = pictureH;
	int bltW = pictureW/16;
	UWORD bltSkip = (width-pictureW)/8;
	
	//Clear a rectangle.
	Hardware->dmacon  = 0x8040; //enable blitter DMA
	BlitWait();
	Hardware->bltcon0 = 0x0100; //no source = clear memory
	Hardware->bltcon1 = 0x0000;
	Hardware->bltdpt  = bitplane;
	Hardware->bltdmod = bltSkip;
	Hardware->bltsize = (UWORD)(bltH*64 + bltW);
}

void B_Blit(PLANEPTR destination, int destX, int destY, APTR source, int srcX, int srcY, int sizeW, int sizeH, PLANEPTR oldBackground) {
	int screenWidth = 320;
	
	destY = destY*2; //?
	
	UWORD shift = (destX % 16) << 12;
	UWORD firstwordmask = 0xFFFF >> shift;
	UWORD lastwordmask = 0xFFFF << (16-shift);
	
	int spriteHeight = sizeH;
	int spriteWidth = sizeW;
	
	int bltH = spriteHeight;
	int bltW = spriteWidth/16;
	
	//Increase blit size by 1 word to account for shifts
	UWORD sourceSkip = (64-spriteWidth)/8 - 1;
	UWORD destinationSkip = (screenWidth/8) - bltW*2 - 1;	
	UWORD blitsize = bltH*64 + bltW +1;
	
	Hardware->dmacon 	= 0x8040; //make sure blit DMA is enabled
	BlitWait(); //wait for the blitter to become available
	
	Hardware->bltcon0	= 0x0FCA | shift; //AB + /AC
	Hardware->bltcon1	= 0x0000 | shift; //shifting the B source
	
	Hardware->bltadat 	= 0xFFFF; //preload with 0xFFFF for cookie cutting;
	Hardware->bltafwm	= firstwordmask;
	Hardware->bltalwm	= lastwordmask;
	
	Hardware->bltapt	= (APTR)((UBYTE *)source + (srcY * (sizeH/16)) + srcX/8);
	Hardware->bltbpt	= (APTR)((UBYTE *)source + (srcY * (sizeH/16)) + srcX/8);
	if(oldBackground == NULL)
		Hardware->bltcpt= (APTR)(destination + (destY * (screenWidth/16)) + destX/8); //if we don't have a background, use the destination
	else
		Hardware->bltcpt= (APTR)oldBackground; //if we do have a background, use it
	Hardware->bltdpt	= (APTR)(destination + (destY * (screenWidth/16)) + destX/8);
	
	Hardware->bltamod	= (UWORD)sourceSkip;
	Hardware->bltbmod	= (UWORD)sourceSkip;
	if(oldBackground == NULL)
		Hardware->bltcmod = (UWORD)destinationSkip; //screen size
	else
		Hardware->bltcmod = (UWORD)0; //we have a background, so don't skip any bytes
	Hardware->bltdmod	= (UWORD)destinationSkip;
	Hardware->bltsize	= blitsize; //execute
}

PLANEPTR B_SaveBackground(PLANEPTR bitplane, int x, int y, int width, int height){
	
	width = width+16; //one extra word for shifts	
	PLANEPTR buffer = AllocMem(MAX(256, width/8 * height/8), MEMF_CHIP|MEMF_CLEAR);
	
	if(buffer == NULL){
		//S_SendString("B_SaveBackground(): FAILED TO ALLOCATE BUFFER!\r\n");
	}
	
	UWORD screenWidth = 320;
	UWORD sourceSkip = (screenWidth/8) - (width/8);
	UWORD destinationSkip = 0;
	UWORD blitsize = height*64 + (width/16);
	
	BlitWait(); //wait for the blitter to become available
	Hardware->dmacon 	= 0x8040; //make sure blit DMA is enabled
	
	Hardware->bltcon0	= 0x09F0; //D = A
	Hardware->bltcon1	= 0x0000; //B is not used
	
	Hardware->bltadat 	= 0xFFFF; //preload with 0xFFFF for cookie cutting;
	Hardware->bltafwm	= 0xFFFF;
	Hardware->bltalwm	= 0xFFFF;
	
	Hardware->bltapt	= (APTR)(bitplane + (y*(screenWidth/8)) + (x/8));
	Hardware->bltdpt	= (APTR)buffer;
	
	Hardware->bltamod	= sourceSkip;
	Hardware->bltdmod	= destinationSkip;
	
	Hardware->bltsize	= blitsize;	
	
	//char buf[100];
	//sprintf(buf, "B_SaveBackground(): saving background at (%d,%d) to %p.\r\n", x, y, buffer);
	//S_SendString(buf);
	
	return buffer;	
}

void B_RestoreBackground(PLANEPTR destination, int destX, int destY, PLANEPTR source, int srcX, int srcY, int sizeW, int sizeH) {
	//copy source to destination (minterm $F0)
	sizeW = sizeW+16; //one extra word for shifts
	
	UWORD screenWidth = 320;
	UWORD sourceSkip = 0;
	UWORD destinationSkip = (screenWidth/8) - (sizeW/8);
	UWORD blitsize = sizeH*64 + (sizeW/16);
	
	BlitWait(); //wait for the blitter to become available
	Hardware->dmacon 	= 0x8040; //make sure blit DMA is enabled
	
	Hardware->bltcon0	= 0x09F0; //D = A
	Hardware->bltcon1	= 0x0000; //B is not used
	
	Hardware->bltadat 	= 0xFFFF; //preload with 0xFFFF for cookie cutting;
	Hardware->bltafwm	= 0xFFFF;
	Hardware->bltalwm	= 0xFFFF;
	
	Hardware->bltapt	= (APTR)source;
	Hardware->bltdpt	= (APTR)(destination + (destY*(screenWidth/8)) + (destX/8));
	
	Hardware->bltamod	= sourceSkip;
	Hardware->bltdmod	= destinationSkip;
	
	Hardware->bltsize	= blitsize;	
	
	//char buf[256];
	//sprintf(buf, "B_RestoreBackground: Restoring background %p to coordinates (%d,%d) of bitplane at %p\r\n", source, destX, destY, destination);
	//S_SendString(buf);
	
}

void WaitForRMBClick() {
	//while ((PortA->ciapra & 0x80) == 0x80) {}; //wait for click...
	//while ((PortA->ciapra & 0x80) != 0x80) {}; //...and release.
	while((Hardware->potinp & 0x0400) == 0x0400) {}; //wait for click...
	while((Hardware->potinp & 0x0400) != 0x0400) {}; //...and release.
}

void B_PlaceBob(PLANEPTR bitplanes[], struct Bob_Sprite *bob){
	
	//Restore the old background
	for(int i=0;i<bob->bitplanes;i++){	
		//char buf[256];
		//sprintf(buf, "Processing bitplane %d", i);
		//S_SendString(buf);
		
		if(bob->background[i] != NULL){
			B_RestoreBackground(bitplanes[i], bob->background_x, bob->background_y, bob->background[i], 0, 0, bob->width, bob->height);
			FreeMem(bob->background[i], MAX(256, (bob->width+16)/8 * (bob->height/8)));
			bob->background[i] = NULL;
		}
				
		//WaitForRMBClick();
		
		//Save the new background
		bob->background[i] = B_SaveBackground(bitplanes[i], bob->position_x, bob->position_y, bob->width, bob->height);
		
		//Place this bob
		B_Blit(bitplanes[i], bob->position_x, bob->position_y, bob->graphics[i], 0, 0, bob->width, bob->height, bob->background[i]);
	}
	bob->background_x = bob->position_x;
	bob->background_y = bob->position_y;
	
}

/* bob sprites */
struct Bob_Sprite *B_AllocateBobSprite(){
	struct Bob_Sprite *buf = AllocMem(sizeof(struct Bob_Sprite), MEMF_PUBLIC);
	buf->position_x = 0;
	buf->position_y = 0;
	buf->width = 0;
	buf->height = 0;
	buf->bitplanes = 0;
	
	for(int i=0;i<5;i++){
		buf->graphics[i] = NULL;
		buf->background[i] = NULL;
	}
	
	return buf;
}

void B_FreeBobSprite(struct Bob_Sprite *bob){
	for(int i=0;i<bob->bitplanes;i++){
		if(bob->background[i] != NULL){
			FreeMem(bob->background[i], MAX(256, (bob->width+16)/8 * (bob->height/8)));
		}
	}
	
	FreeMem(bob, sizeof(struct Bob_Sprite));
}
