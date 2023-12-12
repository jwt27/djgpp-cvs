/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_siglongjmp
_siglongjmp:
	jmp	_longjmp

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
