/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/*
 * File llseek.c.
 *
 * Copyright (C) 2000 Martin Str@"omberg <ams@ludd.luth.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <libc/stubs.h>
#include <stdarg.h>
#include <unistd.h>
#include <dpmi.h>
#include <errno.h>
#include <libc/dosio.h>
#include <sys/fsext.h>
#include <libc/fsexthlp.h>
#include <libc/fd_props.h>

offset_t
llseek( int handle, offset_t offset, int whence )
{
  __dpmi_regs r;
  int has_props;

  __FSEXT_Function *func = __FSEXT_get_function(handle);
  if (func)
  {
    int rv;
    if (__FSEXT_func_wrapper(func, __FSEXT_llseek, &rv, handle, offset, whence))
      return rv;
  }

  has_props = __has_fd_properties(handle);

  /* Directory? If so, we're done. */
  if (has_props && (__fd_properties[handle]->flags & FILE_DESC_DIRECTORY))
    return 0;

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
  return ((((unsigned)r.x.dx ) << 16) + r.x.ax);
}
