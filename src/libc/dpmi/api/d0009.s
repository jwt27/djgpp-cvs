/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	FUNC(___dpmi_set_descriptor_access_rights)
	ENTER

	movl	ARG1, %ebx
	movl	ARG2, %ecx
	DPMI(0x0009)
	xorl	%eax,%eax

       	LEAVE
