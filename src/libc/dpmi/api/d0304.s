/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_free_real_mode_callback)
	ENTER

	movl	ARG1, %eax
	movw	(%eax), %dx
	movw	2(%eax), %cx
	DPMI(0x0304)
	xorl	%eax, %eax

	LEAVE
