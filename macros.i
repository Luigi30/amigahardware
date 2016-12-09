		macro WAITBLIT 
		;a6 = $dff000 or this will surely crash
        tst.w   dmaconr(a6)
.\@:    btst    #6,dmaconr(a6)
        bne     .\@
        endm
	
PUSHW	MACRO
		move.w	\1,-(a7)
		ENDM
	
POPW	MACRO
		move.w	(a7)+,\1
		ENDM
		
PUSHL	MACRO
		move.l	\1,-(a7)
		ENDM
		
POPL	MACRO
		move.l	(a7)+,\1
		ENDM