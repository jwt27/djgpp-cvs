/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_resize_memory)
	ENTER

	movl	ARG1, %eax
	movw	6(%eax), %bx
	movw	4(%eax), %cx
	movw	2(%eax), %si
	movw	(%eax), %di

	DPMI(0x0503)

	movl	ARG1, %edx
	movw	%di, (%edx)
	movw	%si, 2(%edx)
	movw	%cx, 8(%edx)
	movw	%bx, 10(%edx)

	xorl	%eax,%eax

	LEAVE
