/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_set_coprocessor_emulation)
	ENTER

	movl	ARG1, %ebx

	DPMI(0x0e01)
	xorl	%eax,%eax

	LEAVE
