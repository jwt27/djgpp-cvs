/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <dpmi.h>
#include <errno.h>
#include <io.h>
#include <libc/dosio.h>

int
dup2(int fd, int newfd)
{
  __dpmi_regs r;
  if (fd == newfd)
    return newfd;
  r.h.ah = 0x46;
  r.x.bx = fd;
  r.x.cx = newfd;
  __dpmi_int(0x21, &r);
  if (r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  setmode(newfd, __file_handle_modes[fd]);
  return newfd;
}
