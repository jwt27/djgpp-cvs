/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <go32.h>
#include <sys/segments.h>

void
dosmemget(unsigned long offset, size_t length, void *buffer)
{
  movedata((unsigned)_dos_ds,
	    (unsigned)offset,
	    (unsigned)_my_ds(),
	    (unsigned)buffer,
	    length);
}
