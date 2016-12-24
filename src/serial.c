#include "serial.h"

void S_SendByte(UBYTE byte){
	Hardware->serdat = 0x0200 | byte;
	while((Hardware->serdatr & 0x2000) != 0x2000) {}; //wait for transmission
}

void S_SendString(char *msg){
	int length = strlen(msg);

	for(int i=0;i<length;i++){
		S_SendByte((UBYTE)msg[i]);
	}
}