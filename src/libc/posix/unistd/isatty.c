/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <dpmi.h>

int
isatty(int fd)
{
  __dpmi_regs r;
  r.x.ax = 0x4400;
  r.x.bx = fd;
  __dpmi_int(0x21, &r);
  if ((r.x.ax & 0x83) == 0x83)
    return 1;
  return 0;
}
