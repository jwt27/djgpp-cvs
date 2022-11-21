/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
	.data
NaN:
	.long	0xFFC00000

	.text

	.globl	_fmod
_fmod:
	movl	8(%esp), %edx
	andl	$0x7FF00000, %edx
	cmpl	$0x7FF00000, %edx
	je	badarg			/* x is inf or NaN */

	movl	16(%esp), %eax
	andl	$0x7FF00000, %eax
	cmpl	$0x7FF00000, %eax
	je	aby			/* y is inf or NaN */

argok:
	fldl	4(%esp)
	fldl	12(%esp)
	ftst
	fnstsw	%ax
	fxch	%st(1)
	sahf
	jnz	next
	fstp	%st(0)
	jmp	out
next:
	fprem
	fnstsw	%ax
	sahf
	jpe	next
	fstp	%st(1)
out:
	ret

aby:
	movl	16(%esp), %eax		/* inf or NaN */
	testl	$0x000FFFFF, %eax
	jnz	badarg
	movl	4(%esp), %eax
	testl	%eax, %eax
	jnz	badarg

	movl	8(%esp), %eax
	andl	$0x7FF00000, %eax
	cmpl	$0x7FF00000, %eax
	jne	argok			/* y is finite */

badarg:					/* arg is NaN */
	movl	$1, _errno
	flds	NaN
	ret
