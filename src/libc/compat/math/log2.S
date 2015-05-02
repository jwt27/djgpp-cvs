/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
ninf:
	.long	0xFF800000

NaN:
	.long	0xFFC00000

	.globl	_log2
_log2:
	movl	8(%esp), %eax		/* Test for abnormal arg. */
	cmpl	$0x7FF00000, %eax	/* Control flows straight through for */
	jae	abarg			/* normal args: 0 < x < +inf */
	testl	%eax, %eax
	je	verysmall

argok:					/* Proceed with normal log calc. */
	fld1
	fldl	4(%esp)
	fyl2x
	ret

verysmall:				/* exp = max negative */
	movl	4(%esp), %eax
	testl	%eax, %eax
	jne	argok			/* arg > 0; proceed */

zeroarg:
	movl	$2, _errno
	flds	ninf			/* arg = 0; handle separately. */
	ret

abarg:					/* x = negative, +inf, or +NaN */
	ja	badarg			/* arg is negative */
	testl	$0x000FFFFF, %eax
	jnz	badarg
	movl	4(%esp), %eax
	testl	%eax, %eax
	jz	argok

badarg:					/* arg is negative or NaN */
	cmpl	$0x80000000, %eax
	jne	negarg
	movl	4(%esp), %eax		/* Test for abnormal arg. */
	testl	%eax, %eax
	jz	zeroarg

negarg:
	movl	$1, _errno
	flds	NaN
	ret
