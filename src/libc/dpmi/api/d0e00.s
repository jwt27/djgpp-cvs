/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_EBX
#define USE_ESI
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_coprocessor_status)
	ENTER

	DPMI(0x0e00)
	movzwl	%ax,%eax

	LEAVE
