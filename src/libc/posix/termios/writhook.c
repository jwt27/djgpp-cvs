/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <libc/ttyprvt.h>

int (*__libc_write_termios_hook)(int handle, const void *buffer, size_t count,
				 ssize_t *rv) = NULL;
