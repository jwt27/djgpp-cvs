/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <bios.h>
#include <dpmi.h>

int
biosmemory(void)
{
  __dpmi_regs r;
  __dpmi_int(0x12, &r);
  return r.x.ax;
}
