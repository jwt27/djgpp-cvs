/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EDI
#include "dpmidefs.h"

	FUNC(___dpmi_get_multiple_descriptors)
	ENTER

	movl	ARG1, %ecx
	movl	ARG2, %edi
	DPMIce(0x000e)

	movl	ARG1, %eax
	RET

L_error:
	movzwl	%cx, %eax
	negl	%eax

	LEAVE
