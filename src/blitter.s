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
;Preserves all registers

TILE_BLITSIZE = (((16*4)*64))+1
	
	movem.l	a0-a6,-(a7)
	movem.l	d0-d6,-(a7)
	
	move.l	#$dff000,a6	;hw register base
	
	WAITBLIT
	
	;blitter setup
	move.w	#$8040,dmacon(a6)	;blitter dma
	move.w	#$09F0,bltcon0(a6)	;D = A
	move.w	#0,bltcon1(a6)		;not used
	move.l	#$FFFFFFFF,bltafwm(a6) 	;set BLTAFWM and BLTALWM
	move.w	#30,bltamod(a6)		;256px wide = 32 bytes wide, skip 30 bytes per line
	move.w	#38,bltdmod(a6)		;320px wide blitting 32px, skip 38 bytes per line
		
	;TODO: the source offset is calculated wrong
	;Divide by 16 to see what row this tile is on.
	move.w	d2,d5
	ext.l	d5
	lsr		#4,d5		;divide d5 by 16
	
	;d5*2 is the offset into TilemapRowOffsetTable
	lsl		#1,d5
	lea		TilemapRowOffsetTable,a5
	move.w	(a5,d5.l),d6
	add.l	d6,a1

.getTileColumn:
	and.w	#$000F,d2
	lsl		#1,d2		;destY = destY*2
	add.l	d2,a1
	
	;a1 is now a pointer to the tile.
	move.l	a1,bltapt(a6)
	
	;destination pointer
	move.l	a0,d3	;d3 = destination
	mulu	#80,d1
	add.l	d1,d3	;destination += destY * (SCREEN_WIDTH_WORDS * 4)
	lsr		#3,d0	;divide by 8
	add.l	d0,d3	;destination += destX/8
	move.l	d3,bltdpt(a6)
		
	move.w	#TILE_BLITSIZE,bltsize(a6)
	;Hardware->bltsize	= TILE_BLITSIZE; //execute
	
	movem.l	(a7)+,d0-d6
	movem.l	(a7)+,a0-a6

	RTS
	
ScreenYOffsetTable:
	REPT 	208
	dc.w	$50*REPTN
	ENDR
	
TilemapRowOffsetTable:
	REPT	16
	dc.w	$800*REPTN
	ENDR
	
	END