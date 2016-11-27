	XDEF _WFRAME
	XDEF _BlitWait
	
	SECTION CODE
	
_WFRAME:
	;If we're on scanline 244, wait for the *next* scanline 244.
	move.l 	$dff004,d0
	and.l	#$1ff00,d0
	cmp.l 	#244<<8,d0 ;244 = last scanline of the display window
	beq		_WFRAME

_WFRAME_inner:
	move.l 	$dff004,d0
	and.l 	#$1ff00,d0
	cmp.l	#244<<8,d0 ;244 = last scanline of the display window
	bne 	_WFRAME_inner
	RTS
	
_BlitWait:	;wait for the blitter to be available
	tst		$dff002		;compatibility
.waitblit:
	btst	#6,$dff002
	bne		.waitblit
	rts

	END
	