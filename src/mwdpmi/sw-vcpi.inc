; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

; Nothing in this file before this point will survive!

vcpi_go32:
	popw	cs:[vcpi_old_eip - vcpi_go32 + _go32] ; High part is zero
	popw	cs:[vcpi_old_cs - vcpi_go32 + _go32]
	popd	cs:[vcpi_new_eip - vcpi_go32 + _go32]
	popd	cs:[vcpi_new_esp - vcpi_go32 + _go32]
	pushfd
	popd	cs:[vcpi_old_eflags - vcpi_go32 + _go32]
	mov	cs:[vcpi_old_ds - vcpi_go32 + _go32], ds
	mov	cs:[vcpi_old_es - vcpi_go32 + _go32], es
	mov	cs:[vcpi_old_fs - vcpi_go32 + _go32], fs
	mov	cs:[vcpi_old_gs - vcpi_go32 + _go32], gs
	mov	cs:[vcpi_old_ss - vcpi_go32 + _go32], ss
	mov	cs:[vcpi_old_eax - vcpi_go32 + _go32], eax
	mov	cs:[vcpi_old_esi - vcpi_go32 + _go32], esi
	mov	cs:[vcpi_old_esp - vcpi_go32 + _go32], esp ; all popped
	cli
	mov	esi, 0
_vcpi_client_linear = . - 4
	mov	ax, VCPI_MODE_CHANGE
	int	INT_VCPI


vcpi_entry_prot:
	mov	ax, gdt_data32_sel		; Switch to big stack
	mov	ss, ax
	mov	esp, 0x12345678
vcpi_new_esp = . - 4
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

	pushd	0x12345678
vcpi_old_eflags = . - 4

	pushd	0x00001234
vcpi_old_cs = . - 4

	pushd	0x00001234
vcpi_old_eip = . - 4

	pushd	0x00001234
vcpi_old_ds = . - 4
	pushd	0x00001234
vcpi_old_es = . - 4
	pushd	0x00001234
vcpi_old_fs = . - 4
	pushd	0x00001234
vcpi_old_gs = . - 4
	pushd	0x00001234
vcpi_old_ss = . - 4

	pushd	0x12345678
vcpi_old_esp = . - 4

	mov	eax, 0x12345678
vcpi_old_eax = . - 4
	mov	esi, 0x12345678
vcpi_old_esi = . - 4

	pushd	gdt_code32_sel
	pushd	0x12345678
vcpi_new_eip = . - 4
	cld
	.db	OP_OPSIZE
	retf
; ---------------------------------------------------------------------------
.align 16
	.db	128 .dup 0
vcpi_temp_stack = .

vcpi_entry_v86:
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

	mov	eax, cs:[vcpi_old_eax - (vcpi_go32 - _go32)]
	pushd	0x12345678
vcpi_new_eflags = . - 4
	pushd	0x00001234
vcpi_new_cs = . - 4
	pushd	cs:[vcpi_new_eip - (vcpi_go32 - _go32)]
	iretd


vcpi_goreal:
	cli
	clts
	push	eax
	mov	ax, gdt_data16_sel
	mov	ds, ax
	popd	[vcpi_old_eax - (vcpi_go32 - _go32)]
	popd	[vcpi_temp_stack - (vcpi_go32 - _go32) - 24] ; ESP
	popd	[vcpi_temp_stack - (vcpi_go32 - _go32) - 20] ; SS
	popd	[vcpi_temp_stack - (vcpi_go32 - _go32) -  4] ; GS
	popd	[vcpi_temp_stack - (vcpi_go32 - _go32) -  8] ; FS
	popd	[vcpi_temp_stack - (vcpi_go32 - _go32) - 16] ; ES
	popd	[vcpi_temp_stack - (vcpi_go32 - _go32) - 12] ; DS
	popd	[vcpi_new_eip - (vcpi_go32 - _go32)]
	popd	[vcpi_new_cs - (vcpi_go32 - _go32)]
	popd	[vcpi_new_eflags - (vcpi_go32 - _go32)]
	mov	ss, ax
	mov	esp, vcpi_temp_stack - (vcpi_go32 - _go32) - 24
	pushfd				; EFLAGS
	pushd	[_code_seg]		; CS
	pushd	vcpi_entry_v86 - (vcpi_go32 - _go32)
	mov	ax, gdt_core_sel
	mov	ds, ax
	mov	ax, VCPI_MODE_CHANGE
	.opsize
	callf	cs:[_vcpi_entry]
	; never returns
; ---------------------------------------------------------------------------
_vcpi_tss:
	.db	0x68 .dup 0

_vcpi_client:
	.dd	0					; Page table (patched)
	.dd	_gdt_rec				; GDT linear (patched)
	.dd	_idt_rec				; IDT linear (patched)
	.dw	gdt_ldt_sel				; LDT selector
	.dw	gdt_tss_sel				; TSS selector
	.dd	vcpi_entry_prot	- (vcpi_go32 - _go32)	; EIP
	.dw	gdt_code16_sel				; CS

vcpi_go32_end = .
; ---------------------------------------------------------------------------
