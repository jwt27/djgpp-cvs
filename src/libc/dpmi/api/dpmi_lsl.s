/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.file "dpmi_lsl.s"
#include "dpmidefs.h"

	FUNC(___dpmi_get_segment_limit)
	ENTER

	movw	ARG1, %ax
	lsl	%eax, %eax
	jz	1f
	xorl	%eax, %eax		/* Indicate zero limit if not legal */
1:
	LEAVE
