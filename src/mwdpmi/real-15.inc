; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

; This is hooked only when neither XMS nor VCPI is present.

_real_15:
	cmp	ah, BIOS_EXT_MEM_SIZE
	je	@f1
	.db	OP_JMP_FAR
_old15 = .
	.dd	0

@f1:
	xor	ax, ax			; No more extended memory.
	sti
;	clc				; (handled by xor)
	retf	2			; Not quite correct exit, but good
					; enough.
; ---------------------------------------------------------------------------
