/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_state_of_debug_watchpoint)
	ENTER

	movl	ARG1, %ebx

	DPMI(0x0b02)

	movl	ARG2, %edx
	movl	%eax, (%edx)
	xorl	%eax,%eax

	LEAVE
