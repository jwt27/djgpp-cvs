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
#include <libc/fd_props.h>

int
dup2(int fd, int newfd)
{
  __dpmi_regs r;
  if (fd == newfd)
    return newfd;
  /* Reset the text/binary modes, the fsext, and the properties, since
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

  /* The properties may be cleared only after the file has been
     forcibly closed because the file associated with the
     descriptor may be deleted if it has the temporary attribute
     and if newfd is the last reference to it.  */
  if (__has_fd_properties(newfd))
    __clear_fd_properties(newfd);

  /* Copy the fsext hook, the handle modes, and the properties.  */
  __FSEXT_set_function(newfd, __FSEXT_get_function(fd));
  setmode(newfd, __file_handle_modes[fd]);
  if (__has_fd_properties(fd))
    __dup_fd_properties(fd, newfd);

  return newfd;
}
