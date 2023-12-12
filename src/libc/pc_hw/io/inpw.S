/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_inpw
_inpw:
	jmp	_inportw

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
