/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	FUNC(___dpmi_get_segment_base_address)
	ENTER

	movl	ARG1, %ebx
	DPMI(0x0006)

	movl	ARG2, %ebx
	movl	%edx, (%ebx)
	movw	%cx, 2(%ebx)

	xorl	%eax,%eax
	LEAVE
