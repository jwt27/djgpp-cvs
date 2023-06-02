/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
	.global	__toupper
__toupper:
	jmp	_toupper

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
