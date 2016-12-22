	include "macros.i"
	
	XDEF	_UnpackBits
	
	SECTION	PackBits,CODE_P

	;Input 	: a0.L = source
	;		: d2.W = source length in bytes
	;
	;Output	: a1.L = destination
_UnpackBits:
	movem.l	d0-d2,-(sp)
	movem.l	a0-a1,-(sp)

	add.l	a0,d2	;LIMIT := SRCPTR + BYTECOUNT
	BRA.W	.3	;GO TO LOOP START
.1	EXT	d1	;CLEAR HI BYTE OF COUNT
.2	MOVE.B	(A0)+,(A1)+	;COPY A BYTE OF DATA
	DBRA	D1,.1	;LOOP ALL COPY BYTES
.3	CMP.L	D2,A0	;IS SRCPTR >= LIMIT?
	BHS.W	.5		;YES, WE'RE DONE
	MOVE.B	(A0)+,D1	;NO, GET OPCODE
	BPL.W	.1		;0..127 -> COPY 1..128 BYTES
	NEG.B	D1		;-1..-127 -> FILL 2..128 BYTES
	BVS.W	.3		;IGNORE $80 FOR BACKWARD COMPAT
	EXT.W	D1		;CLEAR HI BYTE OF COUNT
	MOVE.B	(A0)+,D0;GET FILL DATA BYTE
.4	MOVE.B	D0,(A1)+;COPY IT TO DST
	DBRA	D1,.4	;LOOP ALL FILL BYTES
	BRA.W	.3		;THEN GET NEXT OPCODE
	
.5	movem.l	(sp)+,a0-a1
	movem.l	(sp)+,d0-d2
	RTS	;done

	END
	