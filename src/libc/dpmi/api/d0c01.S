/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_terminate_and_stay_resident)
	ENTER

	movl	ARG1, %ebx
	movl	ARG2, %edx
	DPMI(0x0c01)
	xorl	%eax,%eax

	LEAVE
