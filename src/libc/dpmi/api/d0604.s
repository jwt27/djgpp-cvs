/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_page_size)
	ENTER

	DPMI(0x0604)

	movl	ARG1, %edx
	movw	%cx, (%edx)
	movw	%bx, 2(%edx)

	xorl	%eax,%eax

	LEAVE
