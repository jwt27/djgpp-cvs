/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_allocate_shared_memory)
	ENTER

	movl	ARG1, %edi
	DPMI(0x0d00)
	xorl	%eax,%eax

	LEAVE
