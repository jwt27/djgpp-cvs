; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

warp_high:
	mov	ax, gdt_core_sel	; Copy code to new location
	mov	es, ax
	mov	edi, SERVER_LINEAR
	xor	esi, esi
	mov	ecx, _code32_end
	.addrsize
	rep
	movsb

	; Patch gdt entries to map to extended memory.
	movw	[gdt + gdt_code32_sel + 2], (SERVER_LINEAR & 0xffff)
	movb	[gdt + gdt_code32_sel + 4], (SERVER_LINEAR >> 16)
	movw	[gdt + gdt_data32_sel + 2], (SERVER_LINEAR & 0xffff)
	movb	[gdt + gdt_data32_sel + 4], (SERVER_LINEAR >> 16)

	; Patch and use high page directory and tables
	; Alignment may be different, so re-align in high memory.
	mov	edi, [_server_physical]
	and	edi, PAGE_SIZE - 1
	add	edi, approximate_temp_page_dir + SERVER_LINEAR
	and	di, 0xffff - (PAGE_SIZE - 1)
	mov	esi, [_page_dir_physical]
	mov	[_page_dir_linear], edi
	call	copy_page

	mov	esi, [_server_page_table_physical]
	mov	[_server_page_table_linear], edi
	call	copy_page

	cmpb	[_vcpi_present], 0
	je	@f1

	sub	edi, 2 * PAGE_SIZE
	mov	si, approximate_temp_page_dir
	shr	si, PAGE_SIZE_LOG
	shl	si, 2
	mov	eax, [si + _vcpi_server_pages]
	mov	ebx, [si + _vcpi_server_pages + 4]
	mov	ecx, [_dosmem_page_table_physical]
	mov	[_vcpi_tss - (vcpi_go32 - _go32) + 0x1c], eax
	mov	[_vcpi_client - (vcpi_go32 - _go32) + 0], eax
	jmp	@f2

@f1:
	; Can't move this page under VCPI and it's specially allocated anyway.
	mov	esi, [_dosmem_page_table_physical]
	call	copy_page
	sub	edi, 3 * PAGE_SIZE
	mov	eax, edi
	sub	eax, SERVER_LINEAR
	add	eax, [_server_physical]
	mov	ebx, eax
	add	ebx, PAGE_SIZE
	mov	ecx, ebx
	add	ecx, PAGE_SIZE

@f2:
	; Install new page directory in EAX,
	; server page table in EBX,
	; dosmem page table in ECX
	mov	[_page_dir_physical], eax
	mov	[_server_page_table_physical], ebx
	mov	[_dosmem_page_table_physical], ecx
	xchg	eax, ecx
	or	al, PT_P | PT_W | PT_U
	.addrsize
	stosd
	mov	eax, ebx
	or	al, PT_P | PT_W | PT_U
	.addrsize
	stosd
	mov	cr3, ecx

	mov	edi, SERVER_LINEAR	; Re-copy low data
	xor	esi, esi
	mov	ecx, (high_data_end + 3) / 4
	.addrsize
	rep
	movsd

; DEBUG SECTION
;	mov	di, temp_high_stack
;@b1:
;	cmp	di, _code32_end
;	je	@f1
;	movb	[di], 0
;	inc	di
;	jmp	@b1
;@f1:
; DEBUG SECTION ENDS

	mov	ax, ss			; Reload all selectors
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax
	mov	esp, temp_high_stack
	.db	OP_JMP_FAR		; Go to extended memory code!
	.dw	_init_server32, gdt_code32_sel
; ---------------------------------------------------------------------------
copy_page:
	mov	ecx, PAGE_SIZE / 4
	.addrsize			; WOW - _four_ prefixes...
	es:
	rep
	movsd
	ret
; ---------------------------------------------------------------------------
