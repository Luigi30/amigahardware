#include "gfx/font.h"

void F_DrawGlyph(UBYTE *bitmap, int x, int y, uint64_t *font, int height, int width, int ascii){
	uint64_t glyph = font[ascii];
	UBYTE glyphLines[8];
	memcpy(glyphLines, &glyph, sizeof(glyph));
	
	for(int i=0;i<height;i++){
		bitmap[(320/8*(y + i)) + (x/8)] = glyphLines[i];
	}
}

void F_PutString(PLANEPTR bitplane, int x, int y, uint64_t *font, int height, int width, char *str){
	for(int i=0;i<strlen(str);i++){
		int newX = x + (width * i);
		F_DrawGlyph(bitplane, newX, y, font, height, width, str[i]);
	}
}

void F_PutColorString(PLANEPTR *bitplanes, int color, int x, int y, uint64_t *font, int height, int width, char *str){
	int bp1 = FALSE;
	int bp2 = FALSE;
	int bp3 = FALSE;
	int bp4 = FALSE;
	int bp5 = FALSE;
	
	//Figure out which combination of bitplanes we need to get this color.
	if((color & 0x01) == 0x01)
		bp1 = TRUE;
	if((color & 0x02) == 0x02)
		bp2 = TRUE;
	if((color & 0x04) == 0x04)
		bp3 = TRUE;
	if((color & 0x08) == 0x08)
		bp4 = TRUE;
	if((color & 0x10) == 0x10)
		bp5 = TRUE;	
	
	for(int i=0;i<strlen(str);i++){
		int newX = x + (width * i);
		
		if(bp1)
			F_DrawGlyph(bitplanes[0], newX, y, font, height, width, str[i]);
		else
			F_DrawGlyph(bitplanes[0], newX, y, font, height, width, 0x20);
		
		if(bp2)
			F_DrawGlyph(bitplanes[1], newX, y, font, height, width, str[i]);
		else
			F_DrawGlyph(bitplanes[1], newX, y, font, height, width, 0x20);
		
		if(bp3)
			F_DrawGlyph(bitplanes[2], newX, y, font, height, width, str[i]);
		else
			F_DrawGlyph(bitplanes[2], newX, y, font, height, width, 0x20);
		
		if(bp4)
			F_DrawGlyph(bitplanes[3], newX, y, font, height, width, str[i]);
		else
			F_DrawGlyph(bitplanes[3], newX, y, font, height, width, 0x20);
		
		if(bp5)
			F_DrawGlyph(bitplanes[4], newX, y, font, height, width, str[i]);
		else
			F_DrawGlyph(bitplanes[4], newX, y, font, height, width, 0x20);
	}	
}