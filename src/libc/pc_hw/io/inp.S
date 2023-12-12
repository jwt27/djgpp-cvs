/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_inp
_inp:
	jmp	_inportb

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
