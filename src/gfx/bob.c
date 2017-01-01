#include "gfx/bob.h"

BlitterObject *BOB_Allocate(){
	BlitterObject *buf = AllocMem(sizeof(BlitterObject), MEMF_PUBLIC);
	buf->position_x = 0;
	buf->position_y = 0;
	buf->width = 0;
	buf->height = 0;
	buf->bitplaneNum = 0;
	buf->mask = NULL;
	buf->bitmap = NULL;
	buf->background = NULL;
	
	return buf;
}

void BOB_Free(BlitterObject *bob){	
	
	if(bob->background != NULL){
		FreeMem(bob->background, MAX(256, (bob->width+16)/8 * (bob->height/8)));
	}
	
	FreeMem(bob, sizeof(BlitterObject));
}