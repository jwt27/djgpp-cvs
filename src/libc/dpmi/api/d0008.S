/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	FUNC(___dpmi_set_segment_limit)
	ENTER

	movl	ARG1, %ebx
	movw	ARG2, %dx
	movw	ARG2h,%cx

	DPMI(0x0008)
	xorl	%eax,%eax

	LEAVE
