; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

move_go32:
	cmpb	[_vcpi_present], 0
	je	@f1

	mov	si, vcpi_go32		; We will use VCPI
	mov	cx, vcpi_go32_end - vcpi_go32
	movw	[_goreal_addr], vcpi_goreal - (vcpi_go32 - _go32)
	jmp	@f2

@f1:					; We will use the raw/xms method
	mov	si, raw_go32
	mov	cx, raw_go32_end - raw_go32
	movw	[_goreal_addr], raw_goreal - (raw_go32 - _go32)
@f2:
	push	ds
	pop	es
	mov	di, _go32
	mov	ax, si
	sub	ax, di
	mov	[_go32_offset], ax
	rep
	movsb
	mov	[_low_memory_end], di

; DEBUG SECTION
;	xor	al, al
;@b1:
;	cmp	di, _go32_end
;	je	@f1
;	stosb
;	jmp	@b1
;@f1:
; DEBUG SECTION ENDS
	ret
; ---------------------------------------------------------------------------
