; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

_dos_major:			; Dos version (3 or more)
	.db	0
_dos_minor:
	.db	0
_vcpi_present:			; VCPI driver present?
	.db	0
_memory_source:			; Where do we allocate from?
	.db	0


.align 2
_dosmem_page_table_seg:		; 4K for VCPI
	.dw	0
_page_dir_seg:			; (temp) Segment of 4K for page directory.
	.dw	0
_server_page_table_seg:		; (temp) Segment of 4K for page table.
	.dw	0
_memory_ext_first:		; First available page in extended memory
	.dw	0
_memory_ext_last:		; Last available page in extended memory
	.dw	0
_memory_total:			; Total number of pages available.
	.dw	0
_server_ext_size:		; Number of bytes for the server in extended
	.dw	0		; memory.
_low_memory_end:		; Byte after last to remain in low memory
	.dw	0


.align 4
_low_code_linear:		; Linear address of low segment
	.dd	0
_dosmem_page_table_physical:	; Physical address of dosmem page table
	.dd	0
_server_page_table_physical:	; Physical address of server page table
	.dd	0
_server_page_table_linear:	; Linear address of server page table in ext
	.dd	0
_server_physical:		; Physical address of server in extended
	.dd	0		; memory.  0 for VCPI-mem.
_page_dir_linear:		; Page directory pte, linear address in ext
	.dd	0
; ---------------------------------------------------------------------------
high_data_end = .
