/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_inb
_inb:
	jmp	_inportb

#include <libc/asmdefs.h>
