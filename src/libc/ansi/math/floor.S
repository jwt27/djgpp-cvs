.data
NaN:
	.long	0xFFC00000

.text
	.globl	_floor
_floor:
	subl	$4, %esp

	movl	12(%esp), %eax
	andl	$0x7FF00000,%eax
	cmpl	$0x7FF00000,%eax
	je	abarg

argok:
	fnstcw	(%esp)
	fwait
	movw	(%esp),%ax
	andw	$0xf3ff,%ax
	orw	$0x0400,%ax
	movw	%ax, 2(%esp)
	fldcww	2(%esp)

	fldl	8(%esp)
	frndint

	fldcww	(%esp)

cleanup:
	addl	$4, %esp
	ret

abarg:
	movl	12(%esp), %eax		/* inf or NaN */
	testl	$0x000FFFFF, %eax
	jnz	badarg
	movl	8(%esp), %eax
	testl	%eax, %eax
	jz	argok

badarg:					/* arg is NaN */
	movl	$1, _errno
	flds	NaN
	jmp	cleanup
