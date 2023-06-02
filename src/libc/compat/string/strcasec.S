/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_strcasecmp
_strcasecmp:
	jmp	___stricmp

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
