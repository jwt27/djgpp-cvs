/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_capabilities)
	ENTER

	movl	ARG2, %edi
	DPMI(0x0401)

	movl	ARG1, %edx
	movl	%eax, (%edx)

	xorl	%eax, %eax

	LEAVE
