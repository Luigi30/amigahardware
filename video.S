	XDEF _WFRAME
	XDEF _BlitWait
	XDEF _ReadJoystick
	
	SECTION CODE
	
_WFRAME:
	;If we're on scanline 244, wait for the *next* scanline 244.
	move.l 	$dff004,d0
	and.l	#$1ff00,d0
	cmp.l 	#252<<8,d0 ;252 = last scanline of the display window
	beq		_WFRAME

_WFRAME_inner:
	move.l 	$dff004,d0
	and.l 	#$1ff00,d0
	cmp.l	#252<<8,d0 ;252 = last scanline of the display window
	bne 	_WFRAME_inner
	RTS
	
_BlitWait:	;wait for the blitter to be available
	tst		$dff002		;compatibility
.waitblit:
	btst	#6,$dff002
	bne		.waitblit
	rts
	
ciaa_pra  = $bfe001
joy1dat   = $dff00c
potgor    = $dff016
bit_joyb1 = 7
bit_joyb2 = 14

_ReadJoystick  btst     #bit_joyb2 & 7, potgor
               seq      d0
               add.w    d0, d0

               btst     #bit_joyb1, ciaa_pra
               seq      d0
               add.w    d0, d0

               move.w   joy1dat, d1
               ror.b    #2, d1
               lsr.w    #6, d1
               and.w    #%1111, d1
               move.b   (.conv, pc, d1.w), d0
               rts

.conv          dc.b      0, 5, 4, 3, 1, 0, 3, 2, 8, 7, 0, 1, 7, 6, 5, 0

	END