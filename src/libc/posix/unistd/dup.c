/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <dpmi.h>
#include <errno.h>
#include <io.h>
#include <libc/dosio.h>

int
dup(int fd)
{
  __dpmi_regs r;
  r.h.ah = 0x45;
  r.x.bx = fd;
  __dpmi_int(0x21, &r);
  if (r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  setmode(r.x.ax, __file_handle_modes[fd]);
  return r.x.ax;
}
