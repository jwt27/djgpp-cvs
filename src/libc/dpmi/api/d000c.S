/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_EDI
#include "dpmidefs.h"

	FUNC(___dpmi_set_descriptor)
	ENTER

	movl	ARG1, %ebx
	movl	ARG2, %edi
	DPMI(0x000c)
	xorl	%eax,%eax

	LEAVE
