/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <dpmi.h>

void delay(unsigned msec)
{
  __dpmi_regs r;
  while (msec)
  {
    unsigned usec;
    unsigned msec_this = msec;
    if (msec_this > 4000)
      msec_this = 4000;
    usec = msec_this * 1000;
    r.h.ah = 0x86;
    r.x.cx = usec>>16;
    r.x.dx = usec & 0xffff;
    __dpmi_int(0x15, &r);
    msec -= msec_this;
  }
}
