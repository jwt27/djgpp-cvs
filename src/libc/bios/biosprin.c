/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <bios.h>
#include <dpmi.h>

int
biosprint(int cmd, int byte, int port)
{
  __dpmi_regs r;
  r.h.ah = cmd;
  r.h.al = byte;
  r.x.dx = port;
  __dpmi_int(0x17, &r);
  return r.h.ah;
}
