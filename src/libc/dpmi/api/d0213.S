/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_set_extended_exception_handler_vector_rm)
	ENTER

	movb	ARG1, %bl
	movl	ARG2, %eax
	movl	(%eax), %edx
	movw	4(%eax), %cx
	DPMI(0x0213)
	xorl	%eax,%eax

	LEAVE
