/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>

	FUNC(_outportb)
	ENTER

	movl	ARG1,%edx
	movl	ARG2,%eax
	outb	%al,%dx

	LEAVE

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
