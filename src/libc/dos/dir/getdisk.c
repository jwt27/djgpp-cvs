/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <dir.h>
#include <dpmi.h>

int
getdisk(void)
{
  __dpmi_regs r;
  r.h.ah = 0x19;
  __dpmi_int(0x21, &r);
  return r.h.al;
}
