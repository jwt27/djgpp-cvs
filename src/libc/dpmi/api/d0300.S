/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_simulate_real_mode_interrupt)
	ENTER

	pushl	%es
	movw	___djgpp_ds_alias, %ax
	movw	%ax, %es

	movl	ARG1, %ebx
	movl	$0, %ecx
	movl	ARG2, %edi
	DPMI(0x0300)
	xorl	%eax, %eax

	LEAVEP(popl %es)
