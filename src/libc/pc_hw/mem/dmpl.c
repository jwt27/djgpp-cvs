/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdint.h>
#include <go32.h>
#include <sys/segments.h>

void
_dosmemputl(const void *buffer, size_t length, unsigned long offset)
{
  fmemcpy1(DP(_dos_ds, offset), buffer, length);
}
