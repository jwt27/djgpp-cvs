/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_ffs
_ffs:
	bsfl	4(%esp), %eax
	jnz	.Lzero
	movl	$0,%eax
.Lzero:
	ret
