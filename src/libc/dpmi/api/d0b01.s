/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_clear_debug_watchpoint)
	ENTER

	movl	ARG1, %ebx
	DPMI(0x0b01)
	xorl	%eax,%eax

	LEAVE
