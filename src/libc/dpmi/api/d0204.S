/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_protected_mode_interrupt_vector)
	ENTER

	movb	ARG1, %bl
	DPMI(0x0204)
	movl	ARG2, %ebx
	movl	%edx, (%ebx)
	movw	%cx, 4(%ebx)
	xorl	%eax,%eax

	LEAVE
