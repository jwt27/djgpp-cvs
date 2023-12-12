/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdint.h>
#include <libc/stubs.h>
#include <go32.h>
#include <sys/segments.h>

void
dosmemget(unsigned long offset, size_t length, void *buffer)
{
  fmemcpy2(buffer, DP(_dos_ds, offset), length);
}
