/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
	.data
NaN:
	.long	0xFFC00000

	.text
	.globl	_modf			/* fract = fmod(x, &integer) */
_modf:
	movl	8(%esp), %eax
	andl	$0x7FF00000, %eax
	cmpl	$0x7FF00000, %eax
	je	abarg			/* x is inf or NaN */

	fldl	4(%esp)			/* x                 */
	fld1				/* 1	    x        */
	fld	%st(1)			/* x        1     x  */
loop:
	fprem				/* fract(x) 1     x  */
	fnstsw	%ax
	sahf
	jpe	loop

	fstp	%st(1)			/* fract(x) x        */
	fxch 	%st(1)			/* x        fract(x) */
	fsub	%st(1), %st		/* int(x)   fract(x) */

	movl	12(%esp), %eax		/* Point to second arg */
	fstpl	(%eax)			/* Store int(x) */
	ret				/* fract(x) */

abarg:					/* arg is inf or NaN */
	movl	$1, _errno
	movl	8(%esp), %eax
	testl	$0x000FFFFF, %eax
	jne	badarg			/* y = NaN */
	movl	4(%esp), %eax
	testl	%eax, %eax
	jnz	badarg

infarg:
	movl	12(%esp), %eax		/* Point to second arg */
	fldl	4(%esp)			/* Load x */
	fstpl	(%eax)			/* Store int(x) */
	flds	NaN
	ret

badarg:
	movl	12(%esp), %eax		/* Point to second arg */
	flds	NaN
	fstl	(%eax)			/* Store int(x) */
	ret
