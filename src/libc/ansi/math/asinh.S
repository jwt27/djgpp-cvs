/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
NaN:
	.long	0xFFC00000

	.globl	_asinh
_asinh:
	/* double asinh(double x) {
	The textbook formula

		asinh(x) = ln(x + sqrt(x*x + 1.))

	involves loss of precision for x small and x negative.	To avoid these
	problems, we split the range by computing

		asinh(x) = -asinh(-x)

	for negative x.	 The textbook formula still involves cancellation, but
	can be put in the form

		asinh(x) = log1p(x + x*x/(sqrt(x*x + 1.) + 1.))
	
	where log1p(x) = log(1+x), a built-in x87 coprocessor function.	 This
	form has no cancellation problems, since all the subexpressions are
	positive.
	*/

	movl	8(%esp), %eax
	movl	%eax, %ecx
	andl	$0x7FF00000, %eax
	cmpl	$0x7FF00000, %eax
	je	abarg


	fldln2				/* ln2			  */
	fldl	4(%esp)			/* x	 ln2		  */
	fabs				/* |x|	 ln2		  */
	fld	%st			/* x	 x    ln2	  */
	fmul	%st, %st		/* x*x	      x	  ln2	  */
	fld1				/* 1	 x*x  x	  ln2	  */
	fadd	%st(1), %st		/* 1+x*x x*x  x	  ln2	  */
	fsqrt				/* sqrt	 x*x  x	  ln2	  */
	fld1				/* 1	 sqrt x*x x   ln2 */
	faddp				/* den	 x*x  x	  ln2	  */
	/* Should be fdivp %st, %st(1) (gas bug) */
	.byte	0xDE, 0xF9		/* quo	 x    ln2	  */
	faddp				/* arg	 ln2		  */

	andl	$0x7FFFFFFF, %eax
	cmpl	$0x3FD6A09E, %eax
	jbe	1f
	fld1
	faddp
	fyl2x				/* logi(x) */
	jmp	2f
1:
		fyl2xp1			/* log1pi(x) */
2:
	testl	%ecx, %ecx		/* test for sign bit */
	jns	3f
		fchs
3:
	ret

abarg:
	movl	8(%esp), %eax		/* inf or NaN */
	testl	$0x000FFFFF, %eax
	jnz	badarg
	movl	4(%esp), %eax
	testl	%eax, %eax
	jnz	badarg

infarg:
	fldl	4(%esp)			/* +/- inf: just load arg */
	ret

badarg:					/* arg is NaN */
	movl	$1, _errno
	flds	NaN
	ret
