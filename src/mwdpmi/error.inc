; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

@b1:
	.db	"Out of extended memory.\r\n",0
out_of_ext_memory:
	mov	dx, @b1
	jmp	bad_exit

@b1:
	.db	"Out of memory.\r\n",0
out_of_memory:
	mov	dx, @b1
; Fall through
; ---------------------------------------------------------------------------
; Print (to stderr) 'Error: ' and then the asciiz string pointed to by
; ds:dx.  Exit with code 1.

bad_exit:
	push	dx
	mov	ah, DOS_WRITE_FILE
	mov	bx, DOS_STDERR_FILENO
	mov	cx, 7
	mov	dx, @f2
	int	INT_DOS
	pop	dx
	mov	bx, dx
@b1:
	cmpb	[bx], 0
	je	@f1
	inc	bx
	jmp	@b1
@f1:
	mov	cx, bx
	sub	cx, dx
	mov	ah, DOS_WRITE_FILE
	mov	bx, DOS_STDERR_FILENO
	int	INT_DOS
	call	_cleanup
	mov	ax, (DOS_EXIT << 8) + 1
	int	INT_DOS
@f2:
	.db	'Error: '
; ---------------------------------------------------------------------------
