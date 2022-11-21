/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
NaN:
	.long	0xFFC00000

	.globl	_fabs
_fabs:
	movl	8(%esp), %eax
	andl	$0x7FF00000, %eax
	cmpl	$0x7FF00000, %eax
	je	abarg

argok:
	fldl	4(%esp)
	fabs
	ret

abarg:
	movl	8(%esp), %eax		/* inf or NaN */
	testl	$0x000FFFFF, %eax
	jnz	badarg
	movl	4(%esp), %eax
	testl	%eax, %eax
	jz	argok

badarg:					/* arg is NaN */
	movl	$1, _errno
	flds	NaN
	ret
