/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>

	FUNC(_inportw)
	ENTER

	movl	ARG1,%edx
	inw	%dx,%ax
	movzwl	%ax,%eax

	LEAVE
