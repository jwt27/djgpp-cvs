/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
NaN:
	.long	0xFFC00000

two:
	.long	0x40000000

	.globl	_tanh
_tanh:

/* double tanh(double x)

	The textbook formula

		tanh(x) = (exp(x) - exp(-x))/(exp(x) + exp(-x))

	has loss of precision for small x.  To avoid these
	problems, if we let y = expm1(2.*x) = exp(2.*x) - 1., where expm1() is
	evaluated using a built-in x87 coprocessor function, we can write the
	hyperbolic tangent as

		tanh(x) = y/(y+2.)

	which has no problems with loss of precision.  Large arguments are
	handled separately to avoid overflow problems and increase speed in
	those ranges.

	The ranges are split and computed as follows:

		tanh(x) = -1.,				   x = [-inf, -32]
		tanh(x) = -expm1(-2.*x)/(expm1(-2.*x)+2.), x = <-32, 0>
		tanh(x) =  expm1(2.*x)/(expm1(2.*x)+2.),   x = [0, 32>
		tanh(x) = +1.,				   x = [32, +inf]

	*/
	movl	8(%esp), %eax
	movl	%eax, %edx		/* Save for later use. */

	andl	$0x7FF00000, %eax
	cmpl	$0x40400000, %eax
	jae	bigarg

argok:
	fldl	4(%esp)
	fabs
	fadd	%st, %st
	fldl2e				/* log2(e)  x			   */
	fmulp				/* xs				   */
	fld	%st			/* xs	    xs			   */
	frndint				/* nint(xs) xs			   */
	fxch	%st(1)			/* xs	    nint		   */
	fsub	%st(1), %st		/* fract    nint		   */
	f2xm1				/* exps-1   nint		   */
	fxch	%st(1)			/* nint	    exps-1		   */
	fld1				/* 1	    nint    exps-1	   */
	fscale				/* scale    nint    exps-1	   */
	fld1				/* 1	    scale   nint    exps-1 */
	/* Should be fsubp %st, %st(1) (gas bug) */
	.byte	0xDE, 0xE9
//	fsubrp	%st, %st(1)		/* scale-1  nint    exps-1	   */
	fxch	%st(2)			/* exps-1   nint    scale-1	   */
	fscale				/* expm	    nint    scale-1	   */
	fstp	%st(1)			/* exp	    scale-1		   */
	faddp				/* exp-1			   */

	fld	%st
	fadds	two
	/* Should be fdivp %st, %st(1) (gas bug) */
	.byte	0xDE, 0xF9

	testl	$0x80000000, %edx	/* Fix up sign. */
	jz	return
	fchs
return:
	ret

bigarg:
	cmpl	$0x7FF00000, %eax
	je	abarg

pnbig:
	testl	$0x80000000, 8(%esp)
	jz	posbig

negbig:
	fld1
	fchs
	ret

posbig:
	fld1
	ret

abarg:
	movl	8(%esp), %eax		/* inf or NaN */
	testl	$0x000FFFFF, %eax
	jnz	badarg
	movl	4(%esp), %eax
	testl	%eax, %eax
	jz	pnbig

badarg:					/* arg is NaN */
	movl	$1, _errno
	flds	NaN
	ret
