/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>

#include <libc/dosio.h>

off_t
lseek(int handle, off_t offset, int whence)
{
  __dpmi_regs r;
  r.h.ah = 0x42;
  r.h.al = whence;
  r.x.bx = handle;
  r.x.cx = offset >> 16;
  r.x.dx = offset & 0xffff;
  __dpmi_int(0x21, &r);
  if (r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  return (r.x.dx << 16) + r.x.ax;
}
