/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>

int bdos(int func, unsigned dx, unsigned al)
{
  union REGS r;
  r.w.dx = dx;
  r.h.ah = func;
  r.h.al = al;
  int86(0x21, &r, &r);
  return r.w.ax;
}
