/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <sys/fsext.h>
#include <libc/dosio.h>
#include <libc/fd_props.h>

off_t
lseek(int handle, off_t offset, int whence)
{
  __dpmi_regs r;
  int has_props;
  
  __FSEXT_Function *func = __FSEXT_get_function(handle);
  if (func)
  {
    int rv;
    if (func(__FSEXT_lseek, &rv, &handle))
      return rv;
  }

  has_props = __has_fd_properties(handle);

  /* POSIX doesn't allow seek on a pipe.  */
  if (has_props && (__fd_properties[handle]->flags & FILE_DESC_PIPE))
  {
    errno = ESPIPE;
    return -1;
  }

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

  if (!has_props ||
      (__fd_properties[handle]->flags & FILE_DESC_DONT_FILL_EOF_GAP) == 0)
  {
    if (offset > 0)
    {
      if (!has_props)
        has_props = (__set_fd_properties(handle, NULL, 0) == 0);
      if (has_props)
        __set_fd_flags(handle, FILE_DESC_ZERO_FILL_EOF_GAP);
    }
    else if (has_props && (whence == SEEK_SET || whence == SEEK_END))
      __clear_fd_flags(handle, FILE_DESC_ZERO_FILL_EOF_GAP);
  }
  return (r.x.dx << 16) + r.x.ax;
}
