#include "game/game.h"

APTR spriteList[8];

struct SpriteCoordinates playerCoordinates;

int scrollXOffset = 0;
int scrollYOffset = 0;

int PLAYER_IS_DEAD;

int scrollingEnabled;

UWORD __chip SPRITE_placeholder[] = {
	0x0000, 0x0000	
};

void F_PutStatusBar(char *str){
	//todo: make variadic function like printf
	
	//pad to 32 characters
	char padded[40];
	strncpy(padded, str, 40);
	padded[39] = 0x00;
	F_PutString(BPStatusBar, 0, 0, FONT_8X8, 8, 8, padded);
	
}

void GAME_Initialize(){
	playerCoordinates.x = 134;
	playerCoordinates.y = 200;
	PLAYER_IS_DEAD = FALSE;
	scrollingEnabled = TRUE;
	
	Hardware->color[17] = 0x0A00;
	Hardware->color[18] = 0x0679;
	Hardware->color[19] = 0x0345;
}

void GAME_UpdateSpriteList(APTR *sprites){
	/* Update sprites in copperlist. */
	copperList[9]		= HIWORD((ULONG)sprites[0]);	/* spr 0 */
	copperList[11]		= LOWORD((ULONG)sprites[0]);
	copperList[13]		= HIWORD((ULONG)sprites[1]);	/* spr 1 */
	copperList[15]		= LOWORD((ULONG)sprites[1]);
	copperList[17]	 	= HIWORD((ULONG)sprites[2]);	/* spr 2 */
	copperList[19]		= LOWORD((ULONG)sprites[2]);
	copperList[21]	 	= HIWORD((ULONG)sprites[3]);	/* spr 3 */
	copperList[23]		= LOWORD((ULONG)sprites[3]);
	copperList[25]	 	= HIWORD((ULONG)sprites[4]);	/* spr 4 */
	copperList[27]		= LOWORD((ULONG)sprites[4]);
	copperList[29]	 	= HIWORD((ULONG)sprites[5]);	/* spr 5 */
	copperList[31]		= LOWORD((ULONG)sprites[5]);
	copperList[33]	 	= HIWORD((ULONG)sprites[6]);	/* spr 6 */
	copperList[35]		= LOWORD((ULONG)sprites[6]);
	copperList[37]	 	= HIWORD((ULONG)sprites[7]);	/* spr 7 */
	copperList[39]		= LOWORD((ULONG)sprites[7]);
}

void GAME_Start(){
	GAME_Initialize();
	
	copperList[65] = (UWORD)0x4200; //enable 4 bitplane display
	
	for(int i=0;i<8;i++){
		spriteList[i] = SPRITE_placeholder;
	}
	
	spriteList[0] = playerSprite1;
	spriteList[1] = playerSprite2;
	
	GAME_FrameLoop();
}

void I_SetSpriteCoordinates(UWORD *sprite, struct SpriteCoordinates coordinates){
	//A single HW sprite.
	
	//Y
	sprite[0] = (coordinates.y << 8);
	sprite[1] = (coordinates.y+32 << 8);

	//X
	sprite[0] = (sprite[0] & 0xFF00) | (coordinates.x & 0x00FF);
	if(coordinates.x > 256) {
		sprite[1] = sprite[1] | 0x0001;
	}
}

void I_SetPairedSpriteCoordinates(UWORD *spriteL, UWORD *spriteR, struct SpriteCoordinates coordinates){
	//Two adjacent HW sprites.
	
	//Y
	spriteL[0] = (coordinates.y << 8);
	spriteR[0] = (coordinates.y << 8);
	spriteL[1] = (coordinates.y+32 << 8);
	spriteR[1] = (coordinates.y+32 << 8);
	
	//X
	spriteL[0] = (spriteL[0] & 0xFF00) | (coordinates.x & 0x00FF);
	spriteR[0] = (spriteR[0] & 0xFF00) | ((coordinates.x+8) & 0x00FF);
	if(coordinates.x > 256) {
		spriteL[1] = spriteL[1] | 0x0001;
		spriteR[1] = spriteR[1] | 0x0001;
	}
}

void GAME_CheckTileEffect(UWORD leftBound, UWORD rightBound){
	//Tiles with ID above 0x80 are water.
	//If the player crosses onto a tile with an ID below 0x80, stop scrolling and kill them.
	if(leftBound < 0x80 || rightBound < 0x80) {
		GAME_PlayerLeftRiver();
	}
}

	
void GAME_FrameLoop() {
	int done = FALSE;
	int framecounter = 0;
	int screenPointerRowOffset = SCREEN_TILE_HEIGHT;
	int tileSourceRow = levelHeight - SCREEN_TILE_HEIGHT;
	int scrollsPerFrame = 1;

	while(!done){
		//S_SendString("FrameLoop(): New frame\r\n");
		
		WFRAME(); //we're out of the drawing area
		framecounter++;
		
		if(PLAYER_IS_DEAD){
			scrollingEnabled = FALSE;
		} else {
			/* Player is alive! */
			
			/* Process inputs. */
			I_UpdatePlayerPosition(&playerCoordinates, I_CheckJoystick1());
			I_SetPairedSpriteCoordinates(playerSprite1, playerSprite2, playerCoordinates);
		}
		
		//F_PutString(BPScreen1, 31, 220, 40, FONT_8X8, 8, 8, "pos bithc");
		
		if(scrollingEnabled)
		{
			for(int i=0;i<scrollsPerFrame;i++){
				scrollYOffset--; //one pixel per frame
			
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
		}
		
		GAME_UpdateSpriteList(spriteList);
		
		CopperPtrs[0] 	= BPScreen1_Interleaved + ((screenPointerRowOffset+2)*2560) + (scrollYOffset*160);
		CopperPtrs[1] 	= BPScreen1_Interleaved+40 + ((screenPointerRowOffset+2)*2560) + (scrollYOffset*160);
		CopperPtrs[2] 	= BPScreen1_Interleaved+80 + ((screenPointerRowOffset+2)*2560) + (scrollYOffset*160);
		CopperPtrs[3] 	= BPScreen1_Interleaved+120 + ((screenPointerRowOffset+2)*2560) + (scrollYOffset*160);
		
		BlitWait();
		
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
		UBYTE playerTileIndex_L = S_PlayerTileIndex_Left(levelTileMap, playerCoordinates, tileSourceRow);
		UBYTE playerTileIndex_R = S_PlayerTileIndex_Right(levelTileMap, playerCoordinates, tileSourceRow);
		
		GAME_CheckTileEffect(playerTileIndex_L, playerTileIndex_R);
		
		if(framecounter % 15 == 0){ //.25 second
			char debug[64];
			sprintf(debug, "X$%2x $Y%2x tX%2d tY%2d tIDX$%2x", 
				playerCoordinates.x, 
				playerCoordinates.y,
				S_TileXOffset(playerCoordinates), 
				S_TileYOffset(playerCoordinates),
				S_PlayerTileIndex_Left(levelTileMap, playerCoordinates, tileSourceRow));
			F_PutStatusBar(debug);
			
			if(I_CheckJoystick1() & INP_UP){
				scrollsPerFrame++;
			}
			else if(I_CheckJoystick1() & INP_DOWN){
				scrollsPerFrame = MAX(1, scrollsPerFrame - 1);
			}
		}
		
		if(framecounter % 60 == 0){ //1 second
			//color++;			
			framecounter = 0;
		}
		
		if(WaitForLMB()){
			done = TRUE;
		}
		
	}
	
}

void GAME_PlayerLeftRiver(){
	PLAYER_IS_DEAD = TRUE;

	I_SetPairedSpriteCoordinates(SPRITE_explode0_L, SPRITE_explode0_R, playerCoordinates);
	spriteList[0] = SPRITE_explode0_L;
	spriteList[1] = SPRITE_explode0_R;
	
	Hardware->color[17] = 0x0A00;
	Hardware->color[18] = 0x0A40;
	Hardware->color[19] = 0x0345;
	
	struct ImageData *testbob = G_FindGraphic(imageList, "TESTBOB");
	//we don't retrieve the info from the iff yet
	//this is a 32x32 4 bitplane image, so 32x128 interleaved
	//B_BlitOverwrite(CopperPtrs[0], 0, 0, testbob->bitmap, 0, 0, 32, 32*NUM_BITPLANES);
	B_Blit(CopperPtrs[0], 0, 0, testbob->bitmap, 0, 0, 32, 32*NUM_BITPLANES, NULL);
}
