/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

#include <bios.h>
#include <dpmi.h>

unsigned
_bios_timeofday(unsigned _cmd, unsigned long *_timeval)
{
  __dpmi_regs r;

  r.h.ah = _cmd;
  if ( _cmd == _TIME_SETCLOCK )
  {
    r.x.cx = *_timeval >> 16;
    r.x.dx = *_timeval & 0xffff;
  }
  __dpmi_int(0x1a, &r);
  if ( _cmd == _TIME_GETCLOCK )
    *_timeval = (r.x.cx << 16) | r.x.dx;
  return r.h.al;
}
