/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_outp
_outp:
	jmp	_outportb
