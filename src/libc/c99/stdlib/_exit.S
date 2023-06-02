/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
	.global	__Exit
__Exit:
	jmp	__exit

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
