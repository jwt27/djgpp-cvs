/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <bios.h>
#include <dpmi.h>

int bioscom(int cmd, char data, int port)
{
  __dpmi_regs r;
  r.h.ah = cmd;
  r.h.al = data;
  r.x.dx = port;
  __dpmi_int(0x14, &r);
  return r.x.ax;
}
