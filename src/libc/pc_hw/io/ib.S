/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>

	FUNC(_inportb)
	ENTER

	movl	ARG1,%edx
	inb	%dx,%al
	movzbl	%al,%eax

	LEAVE
