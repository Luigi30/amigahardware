#include "sprites/sprites.h"

UBYTE S_PlayerTileIndex_Left(UBYTE *tilemap, struct SpriteCoordinates player, int tileSourceRow){
	/* What tile index is the player's top left pixel on? */
	
	int screenX = player.x - PLAYER_SPRITE_L_BOUND; //get the background X pixel
	int screenY = player.y - 44; //get the background Y pixel
	
	//we have the screen pixel coordinate. now figure out what tile that is
	return tilemap[(((tileSourceRow-6 + (screenY/8)) * 20) + (screenX/8))];
}

UBYTE S_PlayerTileIndex_Right(UBYTE *tilemap, struct SpriteCoordinates player, int tileSourceRow){
	/* What tile index is the player's top left pixel on? */
	
	int screenX = player.x + 16 - PLAYER_SPRITE_L_BOUND; //get the background X pixel
	int screenY = player.y - 44; //get the background Y pixel
	
	//we have the screen pixel coordinate. now figure out what tile that is
	return tilemap[(((tileSourceRow-6 + (screenY/8)) * 20) + (screenX/8))];
}

UBYTE S_TileXOffset(struct SpriteCoordinates player){
	int screenX = player.x - PLAYER_SPRITE_L_BOUND; //get the background X pixel
	return screenX/8;
}

UBYTE S_TileYOffset(struct SpriteCoordinates player){
	int screenY = player.y - 44; //get the background Y pixel
	return screenY/16;
}