/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_free_physical_address_mapping)
	ENTER

	movl	ARG1, %eax
	movw	8(%eax), %cx
	movw	10(%eax), %bx

	DPMI(0x0801)
	xorl	%eax,%eax

	LEAVE
