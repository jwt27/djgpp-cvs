; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

; This function cleans up allocated resources.  Note that this function
; should be careful not to call anything that might not be in low memory
; any more.  Also, it shouldn't depend on any registers but CS, EIP, SS,
; and ESP to be sane.

_cleanup:
; ------------------------------
; Restore vector 0x15
	lds	dx, cs:[_old15]
	mov	al, 0x15
	call	cleanup_vector
; ------------------------------
; Restore vector 0x2f
	lds	dx, [_old2f]		; DS now ok.
	mov	al, 0x2f
	call	cleanup_vector
; ------------------------------
; Free any VCPI pages allocated for storing the server.
	mov	bx, _vcpi_server_pages
@b1:
	mov	edx, [bx]
	cmp	dl, -1
	je	@f1
	mov	ax, VCPI_FREE_PAGE
	int	INT_VCPI
	add	bx, 4
	cmp	bx, _vcpi_server_pages_end
	jne	@b1
@f1:
; ------------------------------
; Release EMS handle.

	mov	dx, [_ems_handle]
	or	dx, dx
	jz	@f1
	mov	ah, EMS_DEALLOCATE
	int	INT_EMS
@f1:
; ------------------------------
; Release XMS memory holding the server itself.

	mov	dx, [_xms_server_handle]
	or	dx, dx
	jz	@f1
	mov	ah, XMS_UNLOCK
	callf	[_xms_entry]
	mov	dx, [_xms_server_handle]	; this line needed?
	mov	ah, XMS_FREE
	callf	[_xms_entry]
@f1:
; ------------------------------
; Release DOS memory allocated for VCPI page table.

	mov	ax, [_vcpi_memory_handle]
	or	ax, ax
	je	@f1
	mov	es, ax
	mov	ah, DOS_DEALLOCATE_MEMORY
	int	INT_DOS
@f1:
	ret
; ---------------------------------------------------------------------------
cleanup_vector:
	mov	bx, ds
	or	bx, dx
	je	@f1
	mov	ah, DOS_SET_INT_VEC
	int	INT_DOS
@f1:
	push	cs
	pop	ds
	ret
; ---------------------------------------------------------------------------
_clean_exit:
	xor	al, al
_exit_server:
	push	ax
	call	_cleanup
	pop	ax
	mov	ah, DOS_EXIT
	int	INT_DOS
; ---------------------------------------------------------------------------
