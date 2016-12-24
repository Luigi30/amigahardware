#include "game/game.h"

struct SpriteCoordinates playerCoordinates;

int scrollXOffset = 0;
int scrollYOffset = 0;

void F_PutStatusBar(char *str){
	//todo: make variadic function like printf
	
	//pad to 32 characters
	char padded[40];
	strncpy(padded, str, 40);
	padded[39] = 0x00;
	F_PutString(BPStatusBar, 0, 0, FONT_8X8, 8, 8, padded);
	
}

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
		I_UpdatePlayerPosition(&playerCoordinates, I_CheckJoystick1());
		
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
		UBYTE playerTileIndex = S_PlayerTileIndex_Left(levelTileMap, playerCoordinates, tileSourceRow);
		
		
		if(framecounter % 15 == 0){ //.25 second
			char debug[64];
			sprintf(debug, "X$%2x $Y%2x tX%2d tY%2d tIDX$%2x", 
				playerCoordinates.x, 
				playerCoordinates.y,
				S_TileXOffset(playerCoordinates), 
				S_TileYOffset(playerCoordinates),
				S_PlayerTileIndex_Left(levelTileMap, playerCoordinates, tileSourceRow));
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