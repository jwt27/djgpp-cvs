/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dpmi.h>
#include <io.h>

int
_dos_unlock(int _fd, long _offset, long _length)
{
  __dpmi_regs r;
  r.x.ax = 0x5c01;
  r.x.bx = _fd;
  r.x.cx = _offset >> 16;
  r.x.dx = _offset;
  r.x.si = _length >> 16;
  r.x.di = _length;
  __dpmi_int(0x21, &r);
  if (r.x.flags & 1)
    return r.x.ax;
  return 0;
}
