/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_ESI

#include <libc/asmdefs.h>

	FUNC(_outportsb)	/* port buffer length */
	ENTER

	movl	ARG1,%edx
	movl	ARG2,%esi
	movl	ARG3,%ecx
	cld
	rep
	outsb

	LEAVE
