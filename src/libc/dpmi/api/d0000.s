/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include "dpmidefs.h"

	FUNC(___dpmi_allocate_ldt_descriptors)
	ENTER

	movl	ARG1, %ecx
	DPMI(0x0000)
	movzwl	%ax,%eax

	LEAVE
