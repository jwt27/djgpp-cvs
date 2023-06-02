/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>

	FUNC(_outportw)
	ENTER

	movl	ARG1,%edx
	movl	ARG2,%eax
	outw	%ax,%dx

	LEAVE

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
