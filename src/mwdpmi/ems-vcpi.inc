; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Copyright 1994 CW Sandmann (sandmann@clio.rice.edu)
; Copyright 1991 Y. Shibata
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

; Check that the EMS int is hooked, just in case.  Returns Z=1, if not.
check_vcpi_int:
check_ems_int:
	mov	ax, (DOS_GET_INT_VEC << 8) + INT_EMS
	int	INT_DOS
	mov	ax, es
	or	ax, bx
	ret
; ---------------------------------------------------------------------------
; Allocate an EMS page if possible.  It is not a problem if this fails,
; but VCPI specs say we should do so.

ems_allocate:
	mov	dx, @f1
	mov	ax, (DOS_OPEN_FILE << 8) + 0
	int	INT_DOS
	jc	@f2
	mov	bx, ax
	mov	ah, DOS_CLOSE_FILE
	int	INT_DOS
	call	check_ems_int		; Double check; might be file
	jz	@f2
	mov	bx, 1
	mov	ah, EMS_ALLOCATE	; Allocate Pages (1 Page Only)
	int	INT_EMS
	or	ah, ah
	jne	@f2
	mov	[_ems_handle], dx	; Save to deallocate later
@f2:
	ret

@f1:
	.db	"EMMXXXX0",0		; Device driver name (!)
; ---------------------------------------------------------------------------
check_for_vcpi:
	call	check_vcpi_int
	jz	@f1
	mov	ax, VCPI_PRESENT
	int	INT_VCPI
	or	ah, ah
	je	@f2
@f1:
	ret
@f2:
	; Allocate a permanent page table for VCPI.
	mov	bx, (0x1fff >> 4)
	call	dos_allocate
	jcl	out_of_memory
	mov	[_vcpi_memory_handle], ax
	mov	es, ax
	mov	bx, ax
	add	ax, 0xff		; round to page boundary
	xor	al, al
	mov	[_dosmem_page_table_seg], ax

	add	ax, 0x1000 >> 4		; shrink to just have page
	sub	ax, bx
	mov	bx, ax
	mov	ah, DOS_RESIZE_MEMORY
	int	INT_DOS

	mov	es, [_dosmem_page_table_seg]
	xor	eax, eax
	mov	cx, 0x1000 / 4
	mov	di, ax
	rep
	stosd

	xor	di, di
	mov	si, _vcpi_descriptors
	mov	ax, VCPI_INTERFACE
	int	INT_VCPI
	or	ah, ah
	jne	@f1
	mov	[_vcpi_entry], ebx

	mov	ax, [_page_dir_seg]
	mov	es, ax
	call	vcpi_seg_to_physical
	mov	[_page_dir_physical], eax
	mov	[_vcpi_tss + 0x1c], eax
	mov	[_vcpi_client + 0], eax

	mov	ax, [_dosmem_page_table_seg]
	call	vcpi_seg_to_physical
	mov	[_dosmem_page_table_physical], eax
	or	al, PT_P | PT_W | PT_U
	mov	es:[0], eax

	mov	ax, [_server_page_table_seg]
	call	vcpi_seg_to_physical
	mov	[_server_page_table_physical], eax
	or	al, PT_P | PT_W | PT_U
	mov	es:[4], eax

	incb	[_vcpi_present]

@f1:
	ret
; ---------------------------------------------------------------------------
; In AX=segment, Out Eax=physical address.

vcpi_seg_to_physical:
	shr	ax, PAGE_SIZE_LOG - 4
	mov	cx, ax
	mov	ax, VCPI_GET_PHYMEMADR
	int	INT_VCPI
	mov	eax, edx
	and	ax, 0xffff - (PAGE_SIZE - 1)
	ret
; ---------------------------------------------------------------------------
; Output EDX = #pages

vcpi_capacity:
	mov	ax, VCPI_MEM_CAPACITY
	int	INT_VCPI
	ret
; ---------------------------------------------------------------------------
; Output EAX = max page no

vcpi_max_page:
	mov	ax, VCPI_MAX_PHYMEMADR
 	int	INT_VCPI
	mov	eax, edx
	shr	eax, PAGE_SIZE_LOG
	ret
; ---------------------------------------------------------------------------
; Allocate a VCPI page.  Out CY=Error, EDX=physical address

vcpi_allocate:
	mov	ax, VCPI_ALLOC_PAGE
	int	INT_VCPI
	and	dx, 0xffff - (PAGE_SIZE - 1)
	neg	ah
	ret
; ---------------------------------------------------------------------------
