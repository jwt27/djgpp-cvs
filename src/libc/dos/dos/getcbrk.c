/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <dpmi.h>

int getcbrk(void)
{
  __dpmi_regs r;

  r.h.ah = 0x33;
  r.h.al = 0;
  __dpmi_int(0x21, &r);
  return r.h.dl;
}
