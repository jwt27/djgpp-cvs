/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_free_shared_memory)
	ENTER

	movw	ARG1, %di
	movw	ARG1h, %si

	DPMI(0x0d01)
	xorl	%eax,%eax

	LEAVE
