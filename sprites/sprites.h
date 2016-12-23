#ifndef SPRITES_H
#define SPRITES_H

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <libraries/iffparse.h>

#define PLAYER_SPRITE_L_BOUND 64  /* first left graphic pixel */
#define PLAYER_SPRITE_R_BOUND 208 /* farthest right we can be */

struct SpriteCoordinates {
	UWORD x;
	UWORD y;
};

UBYTE S_PlayerTileIndex_Left(UBYTE *tilemap, struct SpriteCoordinates player, int tileSourceRow);
UBYTE S_PlayerTileIndex_Right(UBYTE *tilemap, struct SpriteCoordinates player, int tileSourceRow);

UBYTE S_TileXOffset(struct SpriteCoordinates player);
UBYTE S_TileYOffset(struct SpriteCoordinates player);

#endif