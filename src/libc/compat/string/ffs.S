/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_ffs
_ffs:
	bsfl	4(%esp), %eax
	jnz	.Lzero
	movl	$-1,%eax
.Lzero:
	incl	%eax
	ret
