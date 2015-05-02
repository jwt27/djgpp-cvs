/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_simulate_real_mode_procedure_retf_stack)
	ENTER

	pushl	%es

	movl	ARG2, %ecx
	testl	%ecx, %ecx
	jz	L_do_call

	/* Set up %esi, %edi to copy data to the PM stack. */
	subl	%ecx, %esp
	subl	%ecx, %esp
	movl	%esp, %edi
	movl	ARG3, %esi

	/* Copy the data. */
	cld
	rep
	movsw

	/* Reload %ecx with the number of words to transfer to RM stack. */
	movl	ARG2, %ecx

L_do_call:
	movw	___djgpp_ds_alias, %es
	xorl	%ebx, %ebx
	movl	ARG1, %edi
	DPMI(0x0301)

	xorl	%eax, %eax

#define RESTORE_ESP	addl ARG2, %esp ; addl ARG2, %esp  /* comma confuses LEAVEP macro. */
	LEAVEP(RESTORE_ESP ; popl %es)
