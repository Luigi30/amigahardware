#include "input.h"

UBYTE I_CheckJoystick1(){
	UWORD input = Hardware->joy1dat; //right port is joystick
	UBYTE out = 0x00; 	//0x01 = up
						//0x02 = right
						//0x04 = down
						//0x08 = left
	
	if(((input & 0x0002) == 0x0002) ^ ((input & 0x0001) == 0x0001))
	{
		out = out | 0x04;
	}
	else if(((input & 0x0200) == 0x0200) ^ ((input & 0x0100) == 0x0100))
	{
		out = out | 0x01;
	}
	else if(input & 0x0002)
	{
		out = out | 0x02;
	}
	else if(input & 0x0200)
	{
		out = out | 0x08;
	}
	
	return out;
}

void I_UpdatePlayerPosition(struct SpriteCoordinates *player, UWORD input){
	if((input & INP_DOWN) == INP_DOWN)
		player->y = MIN(200, player->y + 2);
	else if((input & INP_UP) == INP_UP)
		player->y = MAX(0x80, player->y - 2);
	else if((input & INP_LEFT) == INP_LEFT)
		player->x = MAX(PLAYER_SPRITE_L_BOUND, player->x - 1);
	else if((input & INP_RIGHT) == INP_RIGHT)
		player->x = MIN(PLAYER_SPRITE_R_BOUND, player->x + 1);
}