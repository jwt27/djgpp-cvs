/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>

	FUNC(_htonl)

	movl	4(%esp), %eax
	xchgb	%ah, %al
	rorl	$16, %eax
	xchgb	%ah, %al
	ret
