/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <errno.h>
#include <fcntl.h>
#include <dpmi.h>
#include <libc/dosio.h>

unsigned
_lfn_get_ftime (int fhandle, int which)
{
  unsigned xtime;
  __dpmi_regs r;

  if (which != _LFN_CTIME && which != _LFN_ATIME)
    {
      errno = EINVAL;
      return 0;
    }
  r.x.ax = which == _LFN_ATIME ? 0x5704 : 0x5706;
  r.x.bx = fhandle;
  __dpmi_int (0x21, &r);
  if (r.x.flags & 1)
    {
      errno = __doserr_to_errno (r.x.ax);
      return 0;
    }
  xtime = r.x.dx;
  xtime = (xtime << 16) | r.x.cx;
  return xtime;
}

