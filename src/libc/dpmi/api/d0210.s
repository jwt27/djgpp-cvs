/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_extended_exception_handler_vector_pm)
	ENTER

	movb	ARG1, %bl
	DPMI(0x0210)
	movl	ARG2, %eax
	movl	%edx, (%eax)
	movw	%cx, 4(%eax)
	xorl	%eax,%eax

	LEAVE
