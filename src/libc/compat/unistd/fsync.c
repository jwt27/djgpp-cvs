/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <dpmi.h>
#include <errno.h>
#include <libc/dosio.h>
#include <libc/fd_props.h>

int
fsync(int _fd)
{
  __dpmi_regs r;
  int oerrno = errno;

  /* Directory? If so, fail. */
  if (__get_fd_flags(_fd) & FILE_DESC_DIRECTORY)
  {
    errno = EINVAL;
    return -1;
  }

  r.h.ah = 0x68;
  r.x.bx = _fd;
  __dpmi_int(0x21, &r);
  if ((r.x.flags & 1) && (r.x.ax != 1) && (r.x.ax != 6))
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  errno = oerrno;
  return 0;
}
