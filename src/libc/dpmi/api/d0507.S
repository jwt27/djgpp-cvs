/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_set_page_attributes)
	ENTER

	movl	ARG1, %eax
	movl	(%eax), %esi
	movl	4(%eax), %ecx
	movl	8(%eax), %ebx
	movl	ARG2, %edx

	DPMI(0x0507)

	movl	ARG1, %edx
	movl	%ecx, 4(%edx)

	xorl	%eax,%eax

	LEAVE
