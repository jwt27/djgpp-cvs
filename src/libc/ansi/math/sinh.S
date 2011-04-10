/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
pinf:
	.long	0x7F800000

NaN:
	.long	0xFFC00000

onehalf:
	.float	0.5

	.globl	_sinh
_sinh:

/* double sinh(double x)

	The textbook formula

		sinh(x) = 0.5*(exp(x) - exp(-x))

	has loss of precision for small x.	To avoid these
	problems, if we let y = expm1(|x|) = exp(x|) - 1., where expm1() is
	evaluated using a built-in x87 coprocessor function, we can write the
	hyperbolic sine for positive arguments as

		sinh(x) = 0.5*(y/(y+1.) + y)

	which has no problems with loss of precision.  Large arguments are
	handled separately to increase speed in those ranges.

	The ranges are split and computed as follows:

		sinh(x) = -inf,			x = [-inf, -710]
		sinh(x) = -0.5*(y/(y+1.) + y),	x = <-710, 0>
		sinh(x) =  0.5*(y/(y+1.) + y),	x = [0, 710>
		sinh(x) = inf,			x = [710, +inf]

	errno is set as follows:

	0 = normal completion
	1 = input is NaN
	2 = output is infinite

	*/
	movl	8(%esp), %eax
	movl	%eax, %edx		/* Save for later use. */

	andl	$0x7FFFFFFF, %eax
	cmpl	$0x408633CE, %eax
	jge	bigarg

argok:
	fldl	4(%esp)
	fabs
	fldl2e				/* log2(e)   x			  */
	fmulp				/* xs				  */
	fld	%st			/* xs	     xs			  */
	frndint				/* nint(xs)  xs			  */
	fxch	%st(1)			/* xs	     nint		  */
	fsub	%st(1), %st		/* fract     nint		  */
	f2xm1				/* exps-1    nint		  */
	fxch	%st(1)			/* nint	     exps-1		  */
	fld1				/* 1	     nint    exps-1	  */
	fscale				/* scale     nint    exps-1	  */
	fld1				/* 1	     scale   nint  exps-1 */
	/* Should be fsubp %st, %st(1) (gas bug) */
	.byte	0xDE, 0xE9		/* scale-1   nint    exps-1	  */
	fxch	%st(2)			/* exps-1    nint    scale-1	  */
	fscale				/* expm	     nint    scale-1	  */
	fstp	%st(1)			/* exp	     scale-1		  */
	faddp				/* y=exp-1			  */
	fld1				/* 1	     y			  */
	fadd	%st(1), %st		/* y+1	     y			  */
	fdivr	%st(1), %st		/* y/(y+1)   y			  */
	faddp				/* y+y/(y+1)			  */
	fmuls	onehalf

dosign:
	testl	$0x80000000, %edx	/* Fix up sign. */
	jz	return
	fchs

return:
	ret

bigarg:
	je	edge
	andl	$0x7FF00000, %eax	/* |x| > asinh(DBL_MAX) */
	cmpl	$0x7FF00000, %eax
	je	abarg
	movl	$2, _errno
	jmp	infarg			/* Treat as infinite arg */

edge:					/* |x| is nearly asinh(DBL_MAX) */
	cmpl	$0x8FB9F87D, 4(%esp)
	jbe	argok

infarg:
	flds	pinf
	jmp	dosign

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
