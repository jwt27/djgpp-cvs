/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_raw_mode_switch_addr)
	ENTER

	DPMI(0x0306)
	movl	ARG1, %edx
	movw	%cx, (%edx)
	movw	%bx, 2(%edx)
	movl	ARG2, %edx
	movl	%edi, (%edx)
	movw	%si, 4(%edx)
	xorl	%eax,%eax

	LEAVE
