/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
NaN:
	.long	0x00000000, 0xFFF80000

	.globl	_sincos
_sincos:

	/* void sincos(double x, double *sine, double *cosine); */

	movl	8(%esp), %ecx

	movl	12(%esp), %edx		/* Point to sine. */
	movl	16(%esp), %eax		/* Point to cosine. */

	andl	$0x7FF00000, %ecx	/* Examine exponent of x. */
	cmpl	$0x43E00000, %ecx	/* |x| >= 2^63 */
	jae	bigarg

	fldl	4(%esp)
	fsincos
	fstpl	(%eax)			/* cos */
	fstpl	(%edx)			/* sin */
	ret

bigarg:
	cmpl	$0x7FF00000, %ecx	/* x is INF or NaN. */
	jb	finite
	movl	NaN, %ecx		/* Return -NaN */
	movl	%ecx, (%eax)
	movl	%ecx, (%edx)
	movl	NaN+4, %ecx
	movl	%ecx, 4(%eax)
	movl	%ecx, 4(%edx)
	movl	$1, _errno
	ret

finite:
	fld1
	fstpl	(%eax)			/* cos = 1. */
	fldz
	fstpl	(%edx)			/* sin = 0. */
	ret
