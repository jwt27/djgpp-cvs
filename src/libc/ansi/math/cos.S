/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
NaN:
	.long	0xFFC00000

	.globl	_cos
_cos:
	movl	8(%esp), %eax

	andl	$0x7FF00000, %eax
	cmpl	$0x43E00000, %eax	/* |x| >= 2^63 */
	jae	bigarg

	fldl	4(%esp)
	fcos
	ret

bigarg:
	cmpl	$0x7FF00000, %eax	/* x is INF or NaN. */
	jb	finite
	flds	NaN			/* Result is undefined. */
	movl	$1, _errno
	ret
finite:
	fld1				/* cos = 1. */
	ret
