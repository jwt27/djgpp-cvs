; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

; In this file go data that must be present for cleanup, mode switches,
; or the software interrupt handlers.

_xms_present:			; XMS driver present?
	.db	0

.align 2
_ems_handle:			; Memory handle for EMS block.
	.dw	0
_vcpi_memory_handle:		; For page tables etc.
	.dw	0
_xms_server_handle:		; XMS handle for server itself.
	.dw	0
_go32_offset:
	.dw	0
_code_seg:			; Segment value for low code
	.dw	0,0
_prefixseg:
	.dw	0
_transfer_buffer_seg:		; Segment value for transfer buffer
	.dw	0
_idt_rec:			; Pseudo-descriptor for protected mode IDT
	.dw	0
	.dd	0
real_idt_rec:			; Pseudo-descriptor for protected real IDT
	.dw	0x3ff
	.dd	0
_gdt_rec:			; Pseudo-descriptor for GDT
	.dw	gdt_size - 1
	.dw	gdt, 0


.align 4
_xms_entry:			; Far call address for XMS driver
	.dd	0
_page_dir_physical:		; Page directory pte
	.dd	0
_goreal_addr:			; 16:16 address of prot->real switch
	.dw	0, gdt_code16_sel
_vcpi_entry:			; Protected mode entry point
	.dd	0, gdt_vcpi0_sel
_vcpi_server_pages:		; Physical addresses of server in entended.
	.dd	16 .dup -1	; Let's hope it is enough!
_vcpi_server_pages_end = .

; Strictly speaking this needn't be low.
_gdt_low:
gdt:
	.dd	0, 0				; Zero selector
	.dw	0xffff,0x0000,0x9300,0x00cf	; Core selector
	.dw	0xffff,0x0000,0x9b00,0x0000	; Code 16 selector
	.dw	0xffff,0x0000,0x9300,0x0000	; Data 16 selector
	.dw	0xffff,0x0000,0x9b00,0x00cf	; Code 32 selector
	.dw	0xffff,0x0000,0x9300,0x00cf	; Data 32 selector
	.dw	ldt_selector_count * 8 - 1, 0	; LDT
	.dw	0x8200,0x0040
	.dw	0x0067,0x0000,0x8900,0x0000	; Client's TSS
	.dw	0xffff,0x0400,0xf300,0x0000	; Bios compat.
	.dw	_raw_protect_to_real,gdt_code32_sel ; protect->real gate
	.dw	0xec00, 0
	.dw	_save_state,gdt_code32_sel	; state save gate
	.dw	0xec00, 0
_vcpi_descriptors:
	.dd	0, 0				; VCPI 0
	.dd	0, 0				; VCPI 1
	.dd	0, 0				; VCPI 2
gdt_size = . - gdt
; ---------------------------------------------------------------------------
