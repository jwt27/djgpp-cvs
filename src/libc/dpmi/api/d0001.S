/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	FUNC(___dpmi_free_ldt_descriptor)
	ENTER

	movl	ARG1, %ebx
	DPMI(0x0001)
	xorl	%eax,%eax

	LEAVE
