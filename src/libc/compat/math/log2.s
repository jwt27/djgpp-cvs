/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.globl	_log2
_log2:
	fld1
	fldl	4(%esp)
	fyl2x
	ret
