/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_free_memory)
	ENTER

	movl	ARG1, %edi
	movw	ARG1h, %si

	DPMI(0x0502)
	xorl	%eax,%eax

	LEAVE
