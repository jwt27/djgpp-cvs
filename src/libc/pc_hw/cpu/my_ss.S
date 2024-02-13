/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>

	FUNC(__my_ss)
	movw	%ss,%ax
	movzwl	%ax,%eax
	RET_I

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
