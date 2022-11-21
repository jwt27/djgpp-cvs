/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
	.data
NaN:
	.long	0xFFC00000

	.text
	.globl	_ldexp
_ldexp:

	/* double ldexp(double x, int expon);
		result = x * (2^expon)
	*/

	subl	$8, %esp
	movl	16(%esp), %eax		/* Get high word of x */
	andl	$0x7FF00000, %eax
	cmpl	$0x7FF00000, %eax
	je	abarg			/* x = INF or NaN */

argok:
	fildl	20(%esp)
	fldl	12(%esp)
	fscale
	fstp	%st(1)

	fstl	(%esp)			/* Do integer tests on result. */
	movl	4(%esp), %eax
	testl	$0x7FF00000, %eax	/* Underflow test */
	jz	VerySmall
	andl	$0x7FF00000, %eax	/* Infinity test */
	cmpl	$0x7FF00000, %eax
	je	RangeError

CleanUp:
	addl	$8, %esp
	ret

abarg:
	movl	$1, _errno
	movl	16(%esp), %eax		/* inf or NaN */
	testl	$0x000FFFFF, %eax
	jnz	badarg
	movl	12(%esp), %eax
	testl	%eax, %eax
	jz	argok

badarg:					/* arg is NaN */
	flds	NaN
	jmp	CleanUp

RangeError:
	movl	$2, _errno
	jmp	CleanUp

VerySmall:				/* exp = max negative */
	movl	16(%esp), %eax		/* Get high word of x */
	testl	$0x7FF00000, %eax	/* Signal underflow (ERANGE) */
	jnz	RangeError		/* unless x = 0 */
	movl	12(%esp), %eax
	testl	%eax, %eax
	jnz	RangeError
	jmp	CleanUp
