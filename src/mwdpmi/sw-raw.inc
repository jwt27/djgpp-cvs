; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

; Nothing in this file before this point will survive!

raw_go32:
	popw	cs:[raw_old_eip - raw_go32 + _go32] ; High part is zero
	popw	cs:[raw_old_cs - raw_go32 + _go32]
	popd	cs:[raw_new_eip - raw_go32 + _go32]
	popd	cs:[raw_new_esp - raw_go32 + _go32]
	pushfd
	popd	cs:[raw_old_eflags - raw_go32 + _go32]
	mov	cs:[raw_old_eax - raw_go32 + _go32], eax
	mov	cs:[raw_old_esp - raw_go32 + _go32], esp ; all popped
	mov	cs:[raw_old_ss - raw_go32 + _go32], ss
	cli

	andb	cs:[gdt + gdt_tss_sel + 5], ~2	; Mark not busy

	cmpb	cs:[_xms_present], 0	; Enable A20
	je	@f2
; ------------------------------
	mov	ax, cs			; Using XMS
	mov	ss, ax
	mov	sp, raw_temp_stack
	push	bx
	mov	ah, XMS_LOCAL_ENABLE_A20
	callf	cs:[_xms_entry]
	pop	bx
	jmp	@f1
; ------------------------------
@f2:					; Using ports
	push	ds			; DS=0000, ES=FFFF
	push	es
	xor	ax, ax
	mov	ds, ax
	dec	ax
	mov	es, ax

	call	check_a20		; Already OK?
	je	@f2

	in	al, PS2_PORT_A		; PS/2 method
	or	al, PS2_PORT_A_A20
	.dw	OP_SLOWDOWN
	out	PS2_PORT_A, al
	call	check_a20
	je	@f2

; Taken in part from go32/cwsdpmi
	call	waitkbd			; AT method (keyboard controller)
	mov	al, 0xd1
	out	PORT_KBD_STAT, al
	call	waitkbd
	mov	al, 0xdf		; Patrick
	out	PORT_KBD_DATA, al
	call	waitkbd
	mov	al, 0xff		; Patrick
	out	PORT_KBD_STAT, al	; Patrick
	call	waitkbd			; Patrick
@b1:
	call	check_a20
	jne	@b1

@f2:
	pop	es
	pop	ds
; ------------------------------
@f1:
	lidt	cs:[_idt_rec]
	lgdt	cs:[_gdt_rec]
	mov	eax, cs:[_page_dir_physical]
	mov	cr3, eax
	mov	eax, cr0
	or	eax, (1 << FLAG_CR0_PG) | (1 << FLAG_CR0_PE)
	mov	cr0, eax
	.db	OP_JMP_FAR
	.dw	@f1 - (raw_go32 - _go32), gdt_code16_sel

; Now in _16_ bit protected mode.  Paging is ON.
@f1:
	mov	ax, gdt_data32_sel		; Switch to big stack
	mov	ss, ax
	mov	esp, 0x12345678
raw_new_esp = . - 4

	pushd	0x12345678
raw_old_eflags = . - 4

	pushd	0x00001234
raw_old_cs = . - 4

	pushd	0x00001234
raw_old_eip = . - 4

	.db	OP_OPSIZE
	push	ds
	.db	OP_OPSIZE
	push	es
	.db	OP_OPSIZE
	push	fs
	.db	OP_OPSIZE
	push	gs

	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax

; DEBUG SECTION
;	push	es
;	push	ax
;	push	edi
;	mov	ax, gdt_core_sel
;	mov	es, ax
;	mov	edi, 0xb8000 + 78 * 2
;	mov	ax, 0x1f50	; 'P'
;	.db	OP_ADDRSIZE
;	stosw
;	mov	edi, 0xb8000 + 79 * 2
;	mov	ax, 0x1f20	; ' '
;	.db	OP_ADDRSIZE
;	stosw
;	pop	edi
;	pop	ax
;	pop	es
; DEBUG SECTION ENDS

	mov	ax, gdt_ldt_sel
	lldt	ax
	mov	ax, gdt_tss_sel
	ltr	ax

	pushd	0x00001234
raw_old_ss = . - 4

	pushd	0x12345678
raw_old_esp = . - 4

	mov	eax, 0x12345678
raw_old_eax = . - 4

	pushd	gdt_code32_sel
	pushd	0x12345678
raw_new_eip = . - 4
	cld
	.db	OP_OPSIZE
	retf
; ---------------------------------------------------------------------------
; Check A20 line status.  Z=1, if line ok.  AX destroyed.

check_a20:
	mov	al, es:[0x0010]
	mov	ah, al
	not	al
	xchg	al, [0x0000]
	cmp	ah, es:[0x0010]
	xchg	al, [0x0000]
	ret
; ---------------------------------------------------------------------------
waitkbd:
	push	cx
	xor	cx, cx
@b1:
	in	al, PORT_KBD_STAT
	test	al, 2
	loopnz	@b1
	pop	cx
	ret
; ---------------------------------------------------------------------------
.align 4
	.db	256 .dup 0
raw_temp_stack = .

raw_entry_real:
	; update visible part of SS.
	mov	ax, cs
	mov	ss, ax
	.opsize
	pop	ds
	.opsize
	pop	es
	.opsize
	pop	fs
	.opsize
	pop	gs

	lidt	cs:[real_idt_rec]

	cmpb	cs:[_xms_present], 0
	je	@f1
; ------------------------------
	push	bx			; Using XMS
	mov	ah, XMS_LOCAL_DISABLE_A20
	callf	cs:[_xms_entry]
	pop	bx
	jmp	@f2
; ------------------------------
@f1:					; Using ports
	in	al, PS2_PORT_A		; PS/2 method
	and	al, ~PS2_PORT_A_A20
	.dw	OP_SLOWDOWN
	out	PS2_PORT_A, al
; ------------------------------
@f2:
	.opsize
	pop	ss
	mov	esp, cs:[raw_new_esp - (raw_go32 - _go32)]
	mov	eax, cs:[raw_old_eax - (raw_go32 - _go32)]
	pushd	0x12345678
raw_new_eflags = . - 4
	pushd	0x00001234
raw_new_cs = . - 4
	pushd	cs:[raw_new_eip - (raw_go32 - _go32)]

; DEBUG SECTION
;	push	es
;	push	ax
;	mov	ax, 0xb800
;	mov	es, ax
;	movw	es:[79 * 2], 0x1f00 + 0x52	; 'R'
;	movw	es:[78 * 2], 0x1f00 + 0x20	; ' '
;	pop	ax
;	pop	es
; DEBUG SECTION ENDS
	iretd



raw_goreal:
	cli
	push	eax
	mov	ax, gdt_data16_sel
	mov	ds, ax
	popd	[raw_old_eax - (raw_go32 - _go32)]
	popd	[raw_new_esp - (raw_go32 - _go32)]
	popd	[raw_temp_stack - (raw_go32 - _go32) -  4] ; SS
	popd	[raw_temp_stack - (raw_go32 - _go32) -  8] ; GS
	popd	[raw_temp_stack - (raw_go32 - _go32) - 12] ; FS
	popd	[raw_temp_stack - (raw_go32 - _go32) - 16] ; ES
	popd	[raw_temp_stack - (raw_go32 - _go32) - 20] ; DS
	popd	[raw_new_eip - (raw_go32 - _go32)]
	popd	[raw_new_cs - (raw_go32 - _go32)]
	popd	[raw_new_eflags - (raw_go32 - _go32)]

	; Switch to 16-bit stack.
	mov	ss, ax
	mov	esp, raw_temp_stack - (raw_go32 - _go32) - 20

	mov	eax, cr0
	and	eax, ~((1<<FLAG_CR0_PG) | (1<<FLAG_CR0_TS) | (1<<FLAG_CR0_PE))
	mov	cr0, eax
	.db	OP_JMP_FAR
	.dw	raw_entry_real - (raw_go32 - _go32), 0
raw_goreal_patch = . - 2
; ---------------------------------------------------------------------------
raw_go32_end = .
; ---------------------------------------------------------------------------
