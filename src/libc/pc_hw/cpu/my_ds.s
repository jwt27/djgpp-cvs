/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>

	FUNC(__my_ds)
	movw	%ds,%ax
	movzwl	%ax,%eax
	ret
