/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
NaN:
	.long	0xFFC00000

	.globl	_sqrt
_sqrt:
	movl	8(%esp), %eax		/* Test for negative arg. */
	cmpl	$0x7FF00000, %eax	/* Control flows straight through for */
	jae	abarg			/* normal args: 0 < x < +inf */
argok:
	fldl	4(%esp)
	fsqrt
	ret

abarg:					/* x = negative, inf, or NaN */
	ja	nparg			/* x is non-positive */
	testl	$0x000FFFFF, %eax
	jnz	nparg
	movl	4(%esp), %eax
	testl	%eax, %eax
	jz	argok

nparg:					/* x is non-positive or NaN */
	cmpl	$0x80000000, %eax	/* but filter out x = -0 */
	je	maybezero

badarg:					/* x is definitely <0 or NaN */
	movl	$1, _errno
	flds	NaN
	ret

/* Since -0 compares equal to +0 in the x87, there is no portable way
   for a caller to distinguish between the two.  Thus, we quietly
   set sqrt(-0) = +0 to avoid frivolous error returns. */

maybezero:
	cmpl	$0, 4(%esp)		/* Test for negative zero. */
	jne	badarg
	fldz
	ret
