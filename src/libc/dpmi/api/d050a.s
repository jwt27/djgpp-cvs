/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_memory_block_size_and_base)
	ENTER

	movl	ARG1, %eax
	movw	(%eax), %di
	movw	2(%eax), %si

	DPMI(0x050a)
	xorl	%eax,%eax

	movl	ARG1, %edx
	movw	%di, 4(%edx)
	movw	%si, 6(%edx)
	movw	%cx, 8(%edx)
	movw	%bx, 10(%edx)

	LEAVE
