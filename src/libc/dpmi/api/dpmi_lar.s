/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.file "dpmi_lar.s"
#include "dpmidefs.h"

	FUNC(___dpmi_get_descriptor_access_rights)
	ENTER

	movw	ARG1, %ax
	lar	%eax, %eax
	jz	1f
	xorl	%eax, %eax		/* Indicate zero type if not legal */
1:
	shrl	$8, %eax
	andl	$0xf0ff, %eax

	LEAVE
