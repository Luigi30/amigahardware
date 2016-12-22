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
