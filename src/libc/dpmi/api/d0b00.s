/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_set_debug_watchpoint)
	ENTER

	movl	ARG1, %eax
	movw	8(%eax), %cx
	movw	10(%eax), %bx
	movb	4(%eax), %dl
	movb	ARG2, %dh

	DPMI(0x0b00)

	movl	ARG1, %edx
	movzwl	%bx, %ebx
	movl	%ebx, (%edx)

	xorl	%eax,%eax

	LEAVE
