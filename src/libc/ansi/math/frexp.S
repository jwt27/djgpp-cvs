/* Copyright (C) 2018 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2017 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
pos_NaN:
	.long	0x7FC00000
neg_NaN:
	.long	0xFFC00000

pos_inf:
	.long	0x7F800000
neg_inf:
	.long	0xFF800000

neg_zero:
	.long	0x80000000

onehalf:
	.float	0.5

	.globl	_frexp
_frexp:

	/* double frexp(double value, int *expon);

	returns mantissa in range [0.5, 1), exponent such that
	value = mantissa * (2^expon)
	*/

	movl	12(%esp), %edx		/* Load pointer to expon. */

	movl	8(%esp), %eax
	movl	%eax, %ecx

	andl	$0x7FFFFFFF, %eax
	testl	%eax, %eax		/* Is x = 0 ? */
	jnz	inftest

	movl	4(%esp), %eax
	testl	%eax, %eax
	jnz	inftest

	movl	$0, (%edx)		/* If so, return zeros. */
	testl	%ecx, %ecx		/* Is x = -0 ? */
	jz	poszero

	flds	neg_zero
	ret

poszero:
	fldz
	ret

inftest:
	andl	$0x7FF00000, %eax
	cmpl	$0x7FF00000, %eax
	je	badarg			/* +/- inf or +/- NaN */

argok:
	fldl	4(%esp)
	fxtract
	flds	onehalf
	fmulp

	fxch
	fistpl	(%edx)
	incl	(%edx)
	ret

badarg:					/* arg is +/- inf or +/- NaN */
	movl	$0, (%edx)

	movl	%ecx, %eax

	andl	$0x800FFFFF, %eax
	testl	$0x80000000, %eax
	jnz	negarg

	cmpl	$0x00000000, %eax
	jnz	posNaN

	movl	4(%esp), %eax
	testl	%eax, %eax
	jnz	posNaN

	flds	pos_inf
	ret

posNaN:
	flds	pos_NaN
	ret

negarg:					/* arg is - inf or - NaN */
	cmpl	$0x80000000, %eax
	jnz	negNaN

	movl	4(%esp), %eax
	testl	%eax, %eax
	jnz	negNaN

	flds	neg_inf
	ret

negNaN:
	flds	neg_NaN
	ret
