/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <fcntl.h>
#include <dpmi.h>
#include <errno.h>
#include <io.h>
#include <libc/dosio.h>
#include <libc/fd_props.h>

int
dup2(int fd, int newfd)
{
  __dpmi_regs r;
  if (fd == newfd)
    return newfd;
  __file_handle_set(newfd, __file_handle_modes[fd] ^ (O_BINARY|O_TEXT));
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

  if (__has_fd_properties(newfd))
    __clear_fd_properties(newfd);
  if (__has_fd_properties(fd))
    __dup_fd_properties(fd, newfd);

  return newfd;
}
