/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_real_mode_interrupt_vector)
	ENTER

	movb	ARG1, %bl
	DPMI(0x200)
	movl	ARG2, %ebx
	movw	%dx, (%ebx)
	movw	%cx, 2(%ebx)
	xorl	%eax,%eax

	LEAVE
