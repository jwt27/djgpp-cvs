/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dir.h>
#include <dpmi.h>

int
setdisk(int _drive)
{
  __dpmi_regs r;
  r.h.ah = 0x0e;
  r.h.dl = _drive;
  __dpmi_int(0x21, &r);
  return r.h.al;
}
