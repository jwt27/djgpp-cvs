; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

; If a server is already resident then send the command line to that server.

check_for_resident:
	mov	ax, DPMI_INSTALL_CHECK
	int	INT_DPMI_MULT
	or	ax, ax
	jne	@f1
	mov	eax, [signature]
	sub	di, 14 + 4
	jc	@f1
	cmp	eax, es:[di + 14]
	jne	@f1
	push	es
	push	di
	retf
@f1:
	ret
; ---------------------------------------------------------------------------
