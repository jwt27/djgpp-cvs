/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <go32.h>
#include <libc/farptrgs.h>
#include <pc.h>

int
ScreenMode(void)
{
  return _farpeekb(_dos_ds, 0x449);
}
