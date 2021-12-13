/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdint.h>
#include <libc/stubs.h>
#include <go32.h>
#include <sys/segments.h>
#include <libc/internal.h>

void
dosmemput(const void *buffer, size_t length, unsigned long offset)
{
  movedata((unsigned)_my_ds(),
	    (uintptr_t)buffer,
	    (unsigned)_dos_ds,
	    (unsigned)offset,
	    length);
}
