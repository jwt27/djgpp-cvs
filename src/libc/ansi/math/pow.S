/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
	.data
NaN:
	.long	0xFFC00000

pinf:
	.long	0x7F800000

	.text
one:
	.float	1.

frac:					/* Calculate st(0) - nint(st(0)) */
	fld	%st(0)			/* x		x	*/
	frndint				/* nint(x)	x	*/
	fxch	%st(1)			/* x		nint(x) */
	fsub	%st(1), %st
	ret				/* fract(x)	nint(x) */

Lpow2:					/* Find 2^(fract(st(0))) * 2^st(1) */
	call	frac			/* fract(x)	nint(x) */
	f2xm1				/* 2^fract(x)-1 nint(x) */
	fadds	one			/* 2^fract(x)	nint(x) */
	fscale				/* 2^x			*/
	fstp	%st(1)
	ret

	.globl	_pow
_pow:
	subl	$12, %esp
	movl	20(%esp), %eax
	cmpl	$0x7FF00000, %eax	/* Control flows straight through for */
	jae	abx
	testl	%eax, %eax		/* 0 < x < +inf */
	je	xverysmall

	movl	28(%esp), %eax		/* Now test y. */
	andl	$0x7FF00000, %eax
	cmpl	$0x7FF00000, %eax
	je	aby			/* y is inf or NaN */

argok:
	fldl	24(%esp)
	fldl	16(%esp)
	fyl2x				/* y*log2(x) */
	call	Lpow2
	jmp	test_result

xverysmall:				/* exp = max negative */
	movl	16(%esp), %eax
	testl	%eax, %eax
	jne	argok			/* x > 0; proceed */
	fldl	24(%esp)		/* Load y */

xzero:					/* x == 0; y is in st(0); test y */
	ftst
	fnstsw	%ax
	sahf
	fstp	%st(0)
	ja	zeroresult		/* x = 0 y > 0 */
	jb	infresult		/* x = 0, y < 0 (including y = -inf) */
					/* x = 0, y = 0: */
	fld1				/* return 1 but raise EDOM */ 
	movl	$1, _errno
	jmp	CleanUp

badresult:
	movl	$1, _errno
	flds	NaN
	jmp	CleanUp

zeroresult:
	fldz
	jmp	CleanUp

infresult:
	flds	pinf
	jmp	CleanUp

aby:					/* y is inf or NaN */
	testl	$0x000FFFFF, 28(%esp)
	jnz	badresult		/* Test for y = NaN */
	movl	24(%esp), %eax
	testl	%eax, %eax
	jnz	badresult		/* y = NaN */

	/* At this point y = +/- inf.

	 x  < 0, y= inf			     --> return NaN
	|x| < 1, y=+inf	 or  |x| > 1, y=-inf --> return 0;
	|x| < 1, y=-inf	 or  |x| > 1, y=+inf --> return +inf;
	|x| = 1, y = +/- inf		     --> return NaN
	*/

	movl	20(%esp), %eax		/* Get x */
	testl	$0x80000000, %eax
	jnz	badresult		/* x < 0, y = inf */
	andl	$0x7FF00000, %eax
	movl	16(%esp), %edx
	orw	%dx, %ax		/* OR in low bits of mantissa */
	shrl	$16, %edx
	orw	%dx, %ax

	subl	$0x3FF00000, %eax	/* Integer comparison of x against 1 */

	testl	$0x80000000, 28(%esp)	/* Test sign of y */
	jns	ns
	negl	%eax
ns:
	testl	%eax, %eax
	jz	badresult
	js	zeroresult
	jmp	infresult

abx:
	andl	$0x7FF00000, %eax
	cmpl	$0x7FF00000, %eax
	je	badx			/* x = inf or NaN */

	testl	%eax, %eax		/* x is -finite -- must test for -0 */
	jnz	xltz
	movl	16(%esp), %eax
	testl	%eax, %eax
	fldl	24(%esp)
	jz	xzero			/* x = -0 */

xltz:
	movl	28(%esp), %eax		/* Test y. */
	andl	$0x7FF00000, %eax
	cmpl	$0x7FF00000, %eax
	je	aby			/* y is inf or NaN */

	fldl	24(%esp)		/* x < 0 */
	fldl	16(%esp)
	fabs
	fxch	%st(1)
	call	frac
	ftst
	fnstsw	%ax
	fstp	%st(0)
	sahf
	je	yisint

	fstp	%st(0)			/* x < 0, y != integer */
	fstp	%st(0)
	jmp	badresult

yisint:
	fld	%st
	fistpq	(%esp)
	fxch	%st(1)
	fyl2x
	call	Lpow2
	andl	$1, (%esp)
	jz	yeven
	fchs
yeven:
	jmp	test_result

badx:					/* x = inf or NaN */
	testl	$0x000FFFFF, 20(%esp)
	jnz	badresult		/* x = NaN */
	movl	16(%esp), %eax
	testl	%eax, %eax
	jnz	badresult		/* x = NaN */

	fldl	24(%esp)		/* x = inf; pow(inf, y) = pow(0,-y) */
	fchs
	jmp	xzero

test_result:
	fstl	(%esp)
	movl	4(%esp), %eax
	andl	$0x7FFFFFFF, %eax
	cmpl	$0x7FF00000, %eax	/* Overflow */
	jge	range_error
	cmpl	$0x00080000, %eax	/* Underflow */
	jl	range_error
CleanUp:
	addl	$12, %esp
	ret

range_error:
	movl	$2, _errno
	jmp	CleanUp
