/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdint.h>
#include <go32.h>
#include <sys/segments.h>

void
_dosmemputl(const void *buffer, size_t length, unsigned long offset)
{
  _movedatal((unsigned)_my_ds(),
	     (uintptr_t)buffer,
	     (unsigned)_dos_ds,
	     (unsigned)offset,
	     length);
}
