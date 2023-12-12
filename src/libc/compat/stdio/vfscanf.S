/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
	.global _vfscanf
_vfscanf:
	jmp __doscan

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
