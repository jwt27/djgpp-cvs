/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>

	.text
	FUNC(__rdtsc)
	rdtsc
	RET_I
