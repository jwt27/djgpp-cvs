/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>

	FUNC(_ntohs)

	movl	4(%esp), %eax
	xchgb	%ah, %al
	ret
