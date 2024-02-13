/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>
	
	.text

	FUNC(__clear87)

	fstsw	%ax
	movzwl	%ax, %eax
	fclex
	RET_I


#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
