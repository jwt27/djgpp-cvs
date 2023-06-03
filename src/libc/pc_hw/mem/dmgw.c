/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdint.h>
#include <go32.h>
#include <sys/segments.h>

void
_dosmemgetw(unsigned long offset, size_t length, void *buffer)
{
  fmemcpy2(buffer, DP(_dos_ds, offset), length);
}
