; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

setup_gdt:
	mov	eax, [_low_code_linear]
	add	[_gdt_rec + 2], eax
	add	[gdt + gdt_code16_sel + 2], eax
	add	[gdt + gdt_data16_sel + 2], eax
	add	[gdt + gdt_code32_sel + 2], eax
	add	[gdt + gdt_data32_sel + 2], eax

; This section only for raw/xms
	mov	[raw_goreal_patch], cs

; This section only for VCPI
	add	[_vcpi_client + 4], eax
	add	[_vcpi_client + 8], eax
	movw	[_vcpi_client_linear], _vcpi_client - (vcpi_go32 - _go32)
	add	[_vcpi_client_linear], eax
	movw	[gdt + gdt_tss_sel + 2], _vcpi_tss - (vcpi_go32 - _go32)
	add	[gdt + gdt_tss_sel + 2], eax
	movw	[_vcpi_tss + 0x66], 0x68 ; set I/O permission bitmap

	ret
