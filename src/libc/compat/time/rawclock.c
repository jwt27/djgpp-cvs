/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <time.h>
#include <go32.h>
#include <libc/farptrgs.h>

unsigned long
rawclock(void)
{
  static unsigned long base = 0;
  unsigned long rv;
  rv = _farpeekl(_dos_ds, 0x46c);
  if (base == 0)
    base = rv;
  return rv - base;
}
