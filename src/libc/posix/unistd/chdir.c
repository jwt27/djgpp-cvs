/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <go32.h>
#include <ctype.h>
#include <dpmi.h>
#include <libc/dosio.h>

int
__chdir (const char *dirname)
{
  __dpmi_regs r;

  if (dirname == 0)
  {
    errno = EINVAL;
    return -1;
  }

  if (dirname[0] == 0)
  {
    errno = ENOENT;
    return -1;
  }

  if (dirname[1] != ':' || dirname[2])
  {
    if(_USE_LFN)
      r.x.ax = 0x713b;
    else
      r.h.ah = 0x3b;
    r.x.dx = __tb_offset;
    r.x.ds = __tb_segment;
    _put_path(dirname);
    __dpmi_int(0x21, &r);
    if(r.x.flags & 1)
    {
      errno = __doserr_to_errno(r.x.ax);
      return -1;
    }
  }

  if (dirname[1] == ':')
  {
    /* Change current drive also.  This *will* work if
       the directory change above worked. */
    r.h.ah = 0x0e;
    r.h.dl = (dirname[0] & 0x1f) - 1;
    __dpmi_int(0x21, &r);
  }

  return 0;
}
