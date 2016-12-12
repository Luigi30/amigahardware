	XDEF _B_BlitTileRow
	XDEF _B_BlitTile_ASM
	XREF _BlitWait

	incdir	VINCLUDEOS3ASM:
	include "hardware/custom.i"
	include "macros.i"
	
	SECTION CODE
	
_B_BlitTile_ASM:
;void B_BlitTile_ASM(PLANEPTR destination, int destX, int destY, APTR source, UWORD tileIndex);
;							  A0			   D0		  D1		  A1			D2
;Nukes A0-A1, D0-D4
TILE_BLITSIZE = (16*64) + 1
	
	movem.l	a0-a6,-(a7)
	movem.l	d0-d4,-(a7)
	
	move.l	#$dff000,a6	;hw register base
	lsl		#1,d2		;destY = destY*2
	
	WAITBLIT
	
	;blitter setup
	move.w	#$8040,dmacon(a6)	;blitter dma
	move.w	#$09F0,bltcon0(a6)	;D = A
	move.w	#0,bltcon1(a6)		;not used
	move.l	#$FFFFFFFF,bltafwm(a6) 	;set BLTAFWM and BLTALWM
	move.w	#6,bltamod(a6)		;64px wide = 8 bytes wide, skip 6 bytes per line
	move.w	#38,bltdmod(a6)		;320px wide blitting 32px, skip 38 bytes per line
		
	;All these weird shifts and multiplications need to be replaced by OR and AND
	;Hardware->bltapt	= (UBYTE*)source + ((tileIndex>>2) * 2) + ((tileIndex%4) * 2);
	move.w	d2,d4	;d4 = tileIndex
	
	lsr		#2,d2
	lsl		#1,d2
	add.l	d2,a1	;source += (tileIndex >> 2) * 2
	
	and		#$0003,d4
	lsl		#1,d4
	add.l	d4,a1	;source += (tileIndex % 4) * 2)
	move.l	a1,bltapt(a6)
	
	;destination pointer
	move.l	a0,d3	;d3 = destination
	mulu	#20,d1
	add.l	d1,d3	;destination += destY * SCREEN_WIDTH_WORDS
	lsr		#3,d0	;divide by 8
	add.l	d0,d3	;destination += destX/8
	move.l	d3,bltdpt(a6)
		
	move.w	#TILE_BLITSIZE,bltsize(a6)
	;Hardware->bltsize	= TILE_BLITSIZE; //execute
	
	movem.l	(a7)+,d0-d4
	movem.l	(a7)+,a0-a6

	RTS
	
	END