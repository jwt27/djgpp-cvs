/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_memory_information)
	ENTER

	movl	ARG1, %edi

	DPMI(0x050b)
	xorl	%eax,%eax

	LEAVE
