/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	FUNC(___dpmi_segment_to_descriptor)
	ENTER

	movl	ARG1, %ebx
	DPMI(0x0002)
	movzwl	%ax,%eax

	LEAVE
