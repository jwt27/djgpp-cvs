/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_selector_increment_value)
	ENTER

	DPMI(0x0003)
	movzwl	%ax,%eax

	LEAVE
