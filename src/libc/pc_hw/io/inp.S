/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_inp
_inp:
	jmp	_inportb

#include <libc/asmdefs.h>
