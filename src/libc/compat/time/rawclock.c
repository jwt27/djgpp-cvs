/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <time.h>
#include <go32.h>
#include <libc/farptrgs.h>

unsigned long
rawclock(void)
{
  return _farpeekl(_dos_ds, 0x46c);
}
