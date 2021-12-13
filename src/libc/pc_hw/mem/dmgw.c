/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdint.h>
#include <go32.h>
#include <sys/segments.h>

void
_dosmemgetw(unsigned long offset, size_t length, void *buffer)
{
  _movedataw((unsigned)_dos_ds,
	     (unsigned)offset,
	     (unsigned)_my_ds(),
	     (uintptr_t)buffer,
	     length);
}
