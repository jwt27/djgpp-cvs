/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_outb
_outb:
	jmp	_outportb
