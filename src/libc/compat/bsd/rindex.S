/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/asmdefs.h>
	FUNC(_rindex)
	jmp	_strrchr

#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
