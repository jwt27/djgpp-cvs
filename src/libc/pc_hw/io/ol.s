/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>

	FUNC(_outportl)
	ENTER

	movl	ARG1,%edx
	movl	ARG2,%eax
	outl	%eax,%dx

	LEAVE
