/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_serialize_on_shared_memory)
	ENTER

	movw	ARG1, %di
	movw	ARG1h, %si
	movl	ARG2, %edx

	DPMI(0x0d02)
	xorl	%eax,%eax

	LEAVE
