/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdint.h>
#include <libc/stubs.h>
#include <go32.h>
#include <sys/segments.h>
#include <libc/internal.h>

void
dosmemput(const void *buffer, size_t length, unsigned long offset)
{
  fmemcpy1(DP(_dos_ds, offset), buffer, length);
}
