; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

; Produce a visual effect at top-left corner of the screen.  Works in all
; modes: real/V86, protected 16, protected 32.  Never returns.

are_you_there:
	cli
	cld
	xor	ax, ax
	xor	eax, eax
	inc	ax
	shl	ax, 16
	jnc	@f1

	mov	ax, cs			; 16-bit mode, real/protected
	cmp	ax, cs:[_code_seg]
	je	@f2
; ------------------------------
	mov	ax, gdt_core_sel	; 16-bit protected
	mov	es, ax
	mov	edi, 0x000b8000
@b1:
	.db	OP_ADDRSIZE
	stosw
	dec	di
	dec	di
	inc	ax
	jmp	@b1
; ------------------------------
@f2:					; real/V86 mode
	mov	ax, 0xb800
	mov	ds, ax
	xor	di, di
	cli
@b1:
	incw	[di]
	jmp	@b1
; ------------------------------
@f1:					; 32-bit mode, hopefully protected
	.db	OP_OPSIZE
	mov	ax, gdt_core_sel
	mov	es, ax
	mov	di, 0x8000
	.dw	0x000b
@b1:
	.db	OP_OPSIZE
	stosw
	dec	di
	dec	di
	inc	ax
	jmp	@b1
; ---------------------------------------------------------------------------
