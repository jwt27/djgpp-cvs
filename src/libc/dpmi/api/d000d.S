/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	FUNC(___dpmi_allocate_specific_ldt_descriptor)
	ENTER

	movl	ARG1, %ebx
	DPMI(0x000d)
	xorl	%eax,%eax

	LEAVE
