/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define USE_ESI
#define USE_EBX
#define USE_EDI
#include "dpmidefs.h"

	.text

	FUNC(___dpmi_get_state_save_restore_addr)
	ENTER

	DPMI(0x0305)
	movl	ARG1, %edx
	movw	%cx, (%edx)
	movw	%bx, 2(%edx)
	movl	ARG2, %edx
	movl	%edi, (%edx)
	movw	%si, 4(%edx)
	movzwl	%ax,%eax

	LEAVE
