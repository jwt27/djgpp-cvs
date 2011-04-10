/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
pinf:
	.long	0x7F800000

NaN:
	.long	0xFFC00000

onehalf:
	.float	0.5

	.globl	_cosh
_cosh:

/* double cosh(double x)

	If we let y = exp(|x|), the function is computed as

		cosh(x) = 0.5*(y + 1./y)

	The absolute value is included to ensure that the identity

		cosh(x) = cosh(-x)

	is satisfied exactly.

	errno is set as follows:

	0 = normal completion
	1 = input is NaN
	2 = output is infinite

	*/
	movl	8(%esp), %eax

	andl	$0x7FFFFFFF, %eax
	cmpl	$0x408633CE, %eax
	jge	bigarg

argok:
	fldl	4(%esp)
	fldl2e				/* log2(e)   x		   */
	fmulp				/* log2(e)*x		   */
	fld	%st			/* xs	     xs		   */
	frndint				/* nint(xs)  xs		   */
	fxch	%st(1)			/* xs	     nint(xs)	   */
	fsub	%st(1), %st		/* fract     nint	   */
	f2xm1				/* exps()-1  nint	   */
	fld1				/* 1	     exps-1   nint */
	faddp				/* exps	     nint	   */
	fscale				/* exp	     nint	   */
	fstp	%st(1)			/* y=exp		   */
	fld1				/* 1	     y		   */
	fdiv	%st(1), %st		/* 1/y	     y		   */
	faddp				/* y+1/y		   */
	fmuls	onehalf
	ret

bigarg:
	je	edge
	andl	$0x7FF00000, %eax	/* |x| > acosh(DBL_MAX) */
	cmpl	$0x7FF00000, %eax
	je	abarg
	movl	$2, _errno
	jmp	infarg			/* Treat as infinite arg */

edge:					/* |x| is nearly acosh(DBL_MAX) */
	cmpl	$0x8FB9F87D, 4(%esp)
	jbe	argok
	movl	$2, _errno

infarg:
	flds	pinf
	ret

abarg:
	movl	8(%esp), %eax		/* inf or NaN */
	testl	$0x000FFFFF, %eax
	jnz	badarg
	movl	4(%esp), %eax
	testl	%eax, %eax
	jz	infarg

badarg:					/* arg is NaN */
	movl	$1, _errno
	flds	NaN
	ret
