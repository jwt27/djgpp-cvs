/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_set_real_mode_interrupt_vector)
	ENTER

	movl	ARG2, %ebx
	movw	(%ebx), %dx
	movw	2(%ebx), %cx
	movb	ARG1, %bl
	DPMI(0x201)
	xorl	%eax,%eax

	LEAVE
