/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <bios.h>
#include <dpmi.h>

long
biostime(int cmd, long newtime)
{
  __dpmi_regs r;
  r.h.ah = cmd;
  r.x.cx = newtime >> 16;
  r.x.dx = newtime & 0xffff;
  __dpmi_int(0x1a, &r);
  return (r.x.cx << 16) | r.x.dx;
}
