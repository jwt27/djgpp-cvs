; ---------------------------------------------------------------------------
; Copyright 1995 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------
.copyright "DPMI server 0.1"
.copyright "Copyright 1995 by Morten Welinder."
.copyright "Various parts copyright 1993-1995 Charles Sandmann."
.copyright "Various parts copyright 1993-1995 DJ Delorie."

.include "defines.inc"
; ---------------------------------------------------------------------------
; Start of code to remain in low memory after server moves itself high.

.include "low-data.inc"
.include "real-15.inc"
.include "real-2f.inc"
.include "unload.inc"
.include "cleanup.inc"
.include "debug.inc"
; ---------------------------------------------------------------------------
.align	16				; so that stack ends on para boundary
_server_stack_start:
	.dw	0x200 .dup 0
_server_stack:
.stack
; ---------------------------------------------------------------------------
; ENTRY:	push new ESP, EIP on stack and call far here.
; EXIT:		EAX, EBX, ECX, EDX, ESI, EDI, EBP preserved.
;		EIP, ESP as specified; CS: 32-bit code.
;		DS, ES, FS, GS, SS: 32-bit data.
;		Flags: Interrupts disabled, direction up
;		On protected stack pushed (four bytes each)
;		  EFLAGS, CS, EIP (of instruction after far call)
;		  DS, ES, FS, GS
;		  SS, ESP (as if calling sequence to here did not happen)
.align 4
_go32:

; One and one only of these will remain in low memory.  The code will be
; moved, so some care must be taken not to access absolute addresses after
; the move has been done, or to offset them by (self - _go32).

.include "sw-raw.inc";
.include "sw-vcpi.inc";
; ---------------------------------------------------------------------------
; To go from protected to real:
; 1. Set up stack like "exit" above
; 2. Make a far _jump_ to address specified in [_goreal]

_go32_end:
; ---------------------------------------------------------------------------
; Start of code only present during boot strap and in 32-bit mode.

.include "highdata.inc"
.align 16
.linkcoff "code32.o"

.include "exeparam.inc"

.include "resident.inc"
.include "cpu.inc"
.include "dos.inc"
.include "xms.inc"
.include "ems-vcpi.inc"
.include "gdt.inc"
.include "hook-int.inc"
.include "memory.inc"
.include "page-dir.inc"
.include "warphigh.inc"
.include "movego32.inc"
.include "error.inc"

.start
	mov	ax, cs
	mov	ds, ax
	mov	[_code_seg], ax
	mov	[_prefixseg], es
	cld

	mov	bx, end_of_memory + 0x10f	; 0x100 for psp, 0xf for round
	shr	bx, 4
	mov	ah, DOS_RESIZE_MEMORY
	int	INT_DOS

	xor	eax, eax
	mov	ax, cs
	shl	eax, 4
	mov	[_low_code_linear], eax

	call	check_for_resident
	call	check_cpu
	call	check_dos
	call	setup_page_dir
	call	check_for_xms
	call	ems_allocate
	call	check_for_vcpi
	call	check_cpu_mode
	call	setup_gdt
	call	hook_2f
	call	investigate_memory
	call	move_go32

	pushd	_server_stack
	pushd	@f1
	push	cs
	call	_go32
@f1:					; In 32-bit protected mode
	.db	OP_OPSIZE, OP_ADDRSIZE, OP_JMP_FAR
	.dw	@f1, gdt_code16_sel
@f1:					; In 16-bit protected mode
	call	warp_high
; ---------------------------------------------------------------------------
.bss
; We will set ESP to here in extended memory during the boot process.
.align 16
	.db	0x1000 .dup 0
temp_high_stack = .



	.db	0xfff .dup 0		; Alignment
approximate_temp_page_dir = .		; Dir + server + non-VCPI-1MB
	.db	0x3000 .dup 0

_code32_end = .

end_of_memory:
; ---------------------------------------------------------------------------
