; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

setup_page_dir:
	mov	eax, [_low_code_linear]
	add	eax, approximate_temp_page_dir
	and	ax, 0xffff - (PAGE_SIZE - 1)
	mov	ebx, eax
	shr	ebx, 4
	mov	[_page_dir_physical], eax
	mov	[_page_dir_seg], bx
	mov	es, bx

	add	eax, PAGE_SIZE
	add	bx, PAGE_SIZE >> 4
	mov	[_server_page_table_physical], eax
	mov	[_server_page_table_seg], bx

	add	eax, PAGE_SIZE
	add	bx, PAGE_SIZE >> 4
	mov	[_dosmem_page_table_physical], eax
	mov	[_dosmem_page_table_seg], bx

	xor	di, di
;	mov	eax, [_dosmem_page_table_physical]
	or	al, PT_P | PT_W | PT_U
	stosd
	mov	eax, [_server_page_table_physical]
	or	al, PT_P | PT_W | PT_U
	stosd
	xor	eax, eax
	mov	cx, (0x3000 / 4) - 2
	rep
	stosd

	mov	eax, PT_P | PT_U | PT_W
	mov	es, [_dosmem_page_table_seg]
	xor	di, di
@b1:
	stosd
	add	eax, PAGE_SIZE
	cmp	di, (0x100000 / PAGE_SIZE) * 4
	jne	@b1

	ret
; ---------------------------------------------------------------------------
