/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_processor_exception_handler_vector)
	ENTER

	movb	ARG1, %bl
	DPMI(0x0202)
	movl	ARG2, %ebx
	movl	%edx, (%ebx)
	movw	%cx, 4(%ebx)
	xorl	%eax,%eax

	LEAVE
