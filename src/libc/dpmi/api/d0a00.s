/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_vendor_specific_api_entry_point)
	ENTER

	movl	ARG1, %esi

	pushw	%es
	pushw	%ds
	pushw	%fs
	pushw	%gs
	pushl	%ebp

	movw	$0x0a00, %ax
	int	$0x31

	popl	%ebp
	popw	%gs
	popw	%fs
	popw	%ds

	jnc	L_noerror
	popw	%es
	movw	%ax, ___dpmi_error
	movl	$-1, %eax
	jmp	L_leave
L_noerror:

	movl	ARG2, %edx
	movl	%edi, (%edx)
	movw	%es, 4(%edx)
	popw	%es
	xorl	%eax,%eax

	LEAVE
