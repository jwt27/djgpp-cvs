/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <libc/ttyprvt.h>

ssize_t (*__libc_read_termios_hook)(int handle, void *buffer, size_t count,
				    ssize_t *rv) = NULL;
