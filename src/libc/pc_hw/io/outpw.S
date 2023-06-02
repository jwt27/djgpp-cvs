/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_outpw
_outpw:
	jmp	_outportw

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
