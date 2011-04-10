/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
NaN:
	.long	0xFFC00000

DBL_MAX:
	.long   0xFFFFFFFF, 0x7FEFFFFF

	.globl	_hypot
_hypot:
	movl	8(%esp), %eax
	xorb	%cl, %cl
	andl	$0x7FF00000, %eax
	cmpl	$0x7FE00000, %eax
	jae	abx			/* x = huge, INF or NaN */

testy:
	movl	16(%esp), %eax
	andl	$0x7FF00000, %eax
	cmpl	$0x7FE00000, %eax
	jae	aby			/* y = huge, INF or NaN */

argok:
	fldl	4(%esp)
	fmul	%st, %st
	fldl	12(%esp)
	fmul	%st, %st
	faddp
	fsqrt
	ret

abx:					/* x = huge, INF, or NaN */
	je	aby			/* x = huge but not INF */
	orb	$3, %cl
	movl	8(%esp), %eax
	testl	$0x000FFFFF, %eax
	jnz	badarg			/* x = NaN */
	movl	4(%esp), %eax
	testl	%eax, %eax
	jnz	badarg			/* x = NaN */

aby:
	orb	$1, %cl
	movl	16(%esp), %eax
	andl	$0x7FF00000, %eax
	cmpl	$0x7FE00000, %eax
	jbe	hugey			/* y = huge but not INF */
	movl	16(%esp), %eax
	testl	$0x000FFFFF, %eax
	jnz	badarg			/* y = NaN */
	movl	12(%esp), %eax
	testl	%eax, %eax
	jnz	badarg			/* y = NaN */
	orb	$3, %cl

hugey:
	cmpb	$3, %cl
	je	argok
	jmp	bigarg

badarg:					/* arg is NaN */
	movl	$1, _errno
	flds	NaN
	ret
					/* Both args are finite, but at */
bigarg:					/* least one arg is huge.  Proceed */
	fldl	4(%esp)			/* with caution. */
	fmul	%st, %st
	fldl	12(%esp)
	fmul	%st, %st
	faddp
	fsqrt
	fcoml	DBL_MAX
	fnstsw	%ax
	sahf
	ja	range_error
	ret

range_error:
	movl	$2, _errno
	ret
