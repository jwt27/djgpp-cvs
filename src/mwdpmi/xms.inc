; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

; ---------------------------------------------------------------------------
; Check for the presence of an XMS driver and get its entry point.

check_for_xms:
	mov	ax, XMS_CHECK
	int	INT_XMS
	cmp	al, 0x80
	je	@f1
	ret
@f1:
	mov	ax, XMS_GET_ENTRY
	int	INT_XMS
	mov	[_xms_entry], bx
	mov	[_xms_entry+2], es
	incb	[_xms_present]
	ret
; ---------------------------------------------------------------------------
; Allocate a block of XMS memory.  In DX=size/KB; Out CY=Error, DX=handle

xms_allocate:
	mov	ah, XMS_ALLOCATE
	callf	[_xms_entry]
	dec	ax
	neg	ax
	ret
; ---------------------------------------------------------------------------
; Lock a block of XMS memory.  In DX=handle;  Out CY=Error, DX:BX=linear addr

xms_lock:
	mov	ah, XMS_LOCK
	callf	[_xms_entry]
	dec	ax
	neg	ax
	ret
; ---------------------------------------------------------------------------
