; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

; Determine how much memory we have.  Allocate:
;   VCPI,XMS => Enough for server in extended memory.
;   Otherwise => All extended memory.

investigate_memory:
	mov	eax, _code32_end
	mov	[_server_ext_size], eax

	cmpb	[_vcpi_present], 0
	je	@f1
; ------------------------------
	call	vcpi_capacity
	or	edx, edx		; No VCPI memory, go on...
	je	@f1

	call	vcpi_max_page
	cmp	eax, PAGE_SIZE * 8
	jb	@f2
	mov	ax, PAGE_SIZE * 8 - 1
	mov	ax, -1
@f2:
	mov	[_memory_ext_last], ax

	mov	ecx, [_server_ext_size]
	add	ecx, PAGE_SIZE - 1
	shr	ecx, PAGE_SIZE_LOG
	mov	bx, _vcpi_server_pages
	mov	es, [_server_page_table_seg]
	xor	di, di
@b1:
	cmp	bx, _vcpi_server_pages_end	; This shouldn't happen!
	jel	out_of_ext_memory		; (bogus message)
	call	vcpi_allocate
	jcl	out_of_ext_memory
	mov	[bx], edx
	or	dl, PT_P | PT_W | PT_U
	mov	es:[di], edx
	add	di, 4
	add	bx, 4
	dec	cx
	jne	@b1

	movb	[_memory_source], MEMORY_FROM_VCPI
	ret
; ------------------------------
@f1:
	cmpb	[_xms_present], 0
	je	@f1

	mov	edx, [_server_ext_size]
	add	edx, KB_SIZE - 1 + 0x10
	shr	edx, KB_SIZE_LOG
	call	xms_allocate
	jcl	out_of_ext_memory
	mov	[_xms_server_handle], dx
	call	xms_lock
	add	bx, 0xf			; Make sure the memory is paragraph
	adc	dx, 0			; aligned.  Probably not needed...
	mov	[_server_physical], bx
	mov	[_server_physical + 2], dx

	cmpb	[_vcpi_present], 0	; If not VCPI then we must not have
	jne	@f2			; have been switched into V86 by the
	call	recheck_cpu_mode	; the XMS server.
@f2:
	movb	[_memory_source], MEMORY_FROM_XMS
	jmpl	set_server_mapping
; ------------------------------
@f1:
	mov	si, 0x100000 >> PAGE_SIZE_LOG

	mov	ah, BIOS_EXT_MEM_SIZE
	int	0x15
	shr	ax, (PAGE_SIZE_LOG - KB_SIZE_LOG)
	add	ax, si
	mov	di, ax

	mov	ax, (DOS_GET_INT_VEC << 8) + INT_VDISK
	int	INT_DOS
	cmp	bh, 0xff
	je	@f2
	cmpd	es:[bx + 0x12], 0x53494456	; "VDIS"
	jne	@f2
	cmpb	es:[bx + 0x16], 0x4b		; "K"
	jne	@f2
	mov	esi, es:[bx + 0x2c]
	and	esi, 0x00ffffff
	add	esi, PAGE_SIZE - 1
	shr	esi, PAGE_SIZE_LOG
@f2:
	call	hook_15

; At this point we should have "SI=first page available" and
; "DI=first page beyond available area".

	cmp	si, di
	jael	out_of_ext_memory

	mov	ax, di
	sub	ax, si
	mov	[_memory_total], ax
	mov	ebx, [_server_ext_size]
	add	ebx, PAGE_SIZE - 1
	shr	ebx, PAGE_SIZE_LOG
	add	bx, 10			; Safety margin
	cmp	ax, bx
	jbl	out_of_ext_memory

	mov	[_memory_ext_first], si
	dec	di
	mov	[_memory_ext_last], di

	movzx	esi, si
	shl	esi, PAGE_SIZE_LOG
	mov	[_server_physical], esi

	movb	[_memory_source], MEMORY_FROM_INT15

set_server_mapping:
	mov	eax, [_server_physical]
	mov	es, [_server_page_table_seg]
	xor	di, di
	mov	ecx, [_server_ext_size]
	add	ecx, PAGE_SIZE - 1
	shr	ecx, PAGE_SIZE_LOG
@b1:
	or	al, PT_P | PT_W | PT_U
	stosd
	add	eax, PAGE_SIZE
	loop	@b1
	ret
; ---------------------------------------------------------------------------
