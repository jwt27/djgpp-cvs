/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <fcntl.h>
#include <dpmi.h>
#include <errno.h>
#include <io.h>
#include <sys/fsext.h>
#include <libc/dosio.h>

int
dup2(int fd, int newfd)
{
  __dpmi_regs r;
  if (fd == newfd)
    return newfd;
  /* Reset the text/binary modes and the fsext, since
     DOS function 46h closes newfd.  */
  __file_handle_set(newfd, __file_handle_modes[fd] ^ (O_BINARY|O_TEXT));
  __FSEXT_set_function(newfd, 0);
  r.h.ah = 0x46;
  r.x.bx = fd;
  r.x.cx = newfd;
  __dpmi_int(0x21, &r);
  if (r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }

  /* Copy the fsext hook and the handle modes.  */
  __FSEXT_set_function(newfd, __FSEXT_get_function(fd));
  setmode(newfd, __file_handle_modes[fd]);
  return newfd;
}
