/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_map_device_in_memory_block)
	ENTER

	movl	ARG1, %eax
	movl	(%eax), %esi
	movl	4(%eax), %ecx
	movl	8(%eax), %ebx
	movl	ARG2, %edx

	DPMI(0x0508)
	xorl	%eax,%eax

	LEAVE
