/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_and_set_virtual_interrupt_state)
	ENTER

	movb	ARG1, %al
	andb	$1, %al
	movb	$0x09, %ah
	int	$0x31
	movzbl	%al, %eax

	LEAVE
