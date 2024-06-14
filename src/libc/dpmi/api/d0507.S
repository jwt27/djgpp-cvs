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

	movw	$0x0507, %ax
	int	$0x31

	jnc	L_noerror
	movl	ARG1, %edx

	/*
	 * Check if output ecx (number of modified pages) is valid.
	 * It has to be less than input ecx (total number of pages)
	 * as modification of some pages at this point failed. If
	 * output ecx is invalid then set it to zero as in this case
	 * probably no page was modified by DPMI host (otherwise it
	 * would have set the valid number of modified pages).
	 */
	cmpl	%ecx, 4(%edx)
	ja	L_ecxvalid
	xorl	%ecx, %ecx
L_ecxvalid:

	movl	%ecx, 4(%edx)
	movw	%ax, ___dpmi_error
	movl	$-1, %eax
	jmp	L_leave
L_noerror:

	xorl	%eax,%eax

	LEAVE

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
