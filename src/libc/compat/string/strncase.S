/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
	.global	_strncasecmp
_strncasecmp:
	jmp	_strnicmp
