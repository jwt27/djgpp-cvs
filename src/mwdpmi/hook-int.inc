; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

hook_15:
	mov	ax, (DOS_GET_INT_VEC << 8) | 0x15
	int	INT_DOS
	mov	[_old15], bx
	mov	[_old15+2], es
	mov	ax, (DOS_SET_INT_VEC << 8) | 0x15
	mov	dx, _real_15
	int	INT_DOS
	ret
; ---------------------------------------------------------------------------
hook_2f:
	mov	ax, (DOS_GET_INT_VEC << 8) | 0x2f
	int	INT_DOS
	mov	[_old2f], bx
	mov	[_old2f+2], es
	mov	ax, (DOS_SET_INT_VEC << 8) | 0x2f
	mov	dx, _real_2f
	int	INT_DOS
	ret
; ---------------------------------------------------------------------------
