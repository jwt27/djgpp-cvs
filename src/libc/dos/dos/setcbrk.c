/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <dpmi.h>

int setcbrk(int v)
{
  __dpmi_regs r;

  r.h.ah = 0x33;
  r.h.al = 1;
  r.h.dl = v;
  __dpmi_int(0x21, &r);
  return r.h.dl;
}
