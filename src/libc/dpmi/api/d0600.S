/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_lock_linear_region)
	ENTER

	movl	ARG1, %eax
	movw	8(%eax), %cx
	movw	10(%eax), %bx
	movw	4(%eax), %di
	movw	6(%eax), %si

	DPMI(0x0600)
	xorl	%eax,%eax
	LEAVE
