/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
NaN:
	.long	0xFFC00000

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

	testl	%eax, %eax		/* Is x = 0 ? */
	jnz	inftest

	movl	4(%esp), %eax
	testl	%eax, %eax
	jnz	inftest
	
	movl	$0, (%edx)		/* If so, return zeros. */
	fldz
	ret

inftest:
	andl	$0x7FF00000, %eax
	cmpl	$0x7FF00000, %eax
	je	badarg			/* +/- inf or NaN */

argok:
	fldl	4(%esp)
	fxtract
	flds	onehalf
	fmulp

	fxch
	fistpl	(%edx)
	incl	(%edx)
	ret

badarg:					/* arg is NaN */
	movl	$1, _errno
	movl	$0, (%edx)
	flds	NaN
	ret
