/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EDI
#include <libc/asmdefs.h>

	FUNC(_inportsl)		/* port buffer length */
	ENTER

	movl	ARG1,%edx
	movl	ARG2,%edi
	movl	ARG3,%ecx
	cld
	rep
	insl

	LEAVE
