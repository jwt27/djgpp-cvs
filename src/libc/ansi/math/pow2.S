/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
NaN:
	.long	0xFFC00000

pinf:
	.long	0x7F800000

	.globl	___pow2
___pow2:
	movl	8(%esp), %eax		/* Test for special cases. */
	andl	$0x7FFFFFFF, %eax
	cmpl	$0x40900000, %eax
	jge	bigarg			/* normal args: */
					/* 0 < |x| <= log2(DBL_MAX) */
argok:					/* log2(DBL_MAX) = 0x4090000000000000 */
	fldl	4(%esp)
	fld	%st			/* xs	    xs		  */
	frndint				/* nint(xs) xs		  */
	fxch	%st(1)			/* xs	    nint(xs)	  */
	fsub	%st(1), %st		/* fract    nint	  */
	f2xm1				/* exps()-1 nint	  */
	fld1				/* 1	    exps-1   nint */
	faddp				/* exps	    nint	  */
	fscale				/* exp	    nint	  */
	fstp	%st(1)			/* exp			  */
	ret

bigarg:
	andl	$0x7FF00000, %eax	/* |x| > log2(DBL_MAX) */
	cmpl	$0x7FF00000, %eax
	je	abarg
	movl	$2, _errno		/* |x| is really big, but finite */
	jmp	argok

abarg:					/* x = +/-inf, or +NaN */
	testl	$0x000FFFFF, 8(%esp)
	jnz	badarg
	movl	4(%esp), %eax
	testl	%eax, %eax
	jnz	badarg

infarg:					/* |x| = inf */
	testl	$0x80000000, 8(%esp)
	jz	posinf

neginf:
	fldz
	ret

posinf:
	flds	pinf
	ret

badarg:					/* arg is NaN */
	movl	$1, _errno
	flds	NaN
	ret
