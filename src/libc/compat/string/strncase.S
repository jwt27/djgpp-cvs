/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_strncasecmp
_strncasecmp:
	jmp	___strnicmp

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
