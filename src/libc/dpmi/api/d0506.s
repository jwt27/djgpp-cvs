/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_page_attributes)
	ENTER

	movl	ARG1, %eax
	movl	(%eax), %esi
	movl	4(%eax), %ecx
	movl	8(%eax), %ebx
	movl	ARG2, %edx

	DPMI(0x0506)
	xorl	%eax,%eax

	LEAVE
