/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>

	FUNC(___dpmi_yield)

	movw	$0x1680, %ax
	int	$0x2f

	ret
