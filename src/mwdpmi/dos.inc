; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

; Check that we have Dos 3 or better.  If not, then exit.

check_dos:
	mov	ah, DOS_GET_VERSION
	int	INT_DOS
	cmp	al, 3
	jb	@f1
	mov	[_dos_major], al
	mov	[_dos_minor], ah
	cmp	al, 5
	jb	@f3
	mov	ax, DOS_GET_ALLOCATION_STRATEGY
	int	INT_DOS
	mov	[_allocation_strategy], ax
	mov	ax, DOS_GET_UMB_STATUS
	int	INT_DOS
	mov	ah, 0
	mov	[_umb_status], ax
	ret

@f3:
	movb	[dos_link_umb], OP_NEAR_RET
	movb	[dos_unlink_umb], OP_NEAR_RET
	ret
@f1:
	mov	dx, @f2
	jmpl	bad_exit
@f2:
	.db	"This program needs Dos 3 or better.\r\n"
	.db	0
; ---------------------------------------------------------------------------
; Allocate a block of memory.  In BX=Size/16, Out AX=Segment, CY=Error

dos_allocate:
	call	dos_link_umb
	mov	ah, DOS_ALLOCATE_MEMORY
	int	INT_DOS
	call	dos_unlink_umb
	ret
; ---------------------------------------------------------------------------
; Link UMBs into Dos' memory chain and make Dos select them by default.
; Preserves BX.

dos_link_umb:
	push	bx
	mov	ax, DOS_SET_ALLOCATION_STRATEGY
	mov	bx, 0x0080 ; first fit, high then low
	int	INT_DOS
	mov	ax, DOS_SET_UMB_STATUS
	mov	bx, 0x0001
	int	INT_DOS
	pop	bx
	ret
; ---------------------------------------------------------------------------
; Restores allocation strategy and umb status.  Preserves AX, and CY

dos_unlink_umb:
	push	ax
	pushf
	mov	ax, DOS_SET_UMB_STATUS
	mov	bx, 0x1234
_umb_status = . - 2
	int	INT_DOS
	mov	ax, DOS_SET_ALLOCATION_STRATEGY
	mov	bx, 0x1234
_allocation_strategy = . - 2
	int	INT_DOS
	popf
	pop	ax
	ret
; ---------------------------------------------------------------------------
