/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <io.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <fcntl.h>
#include <libc/dosio.h>
 
int
_chmod(const char *filename, int func, ...)
{
  __dpmi_regs r;

  if(_USE_LFN) {
    r.x.ax = 0x7143;
    r.h.bl = func;			/* Get or Put */
  } else
    r.x.ax = 0x4300 + func;
  _put_path(filename);
  if (func == 1)
    r.x.cx = *(&func + 1);		/* Value to set */
  r.x.dx = __tb_offset;
  r.x.ds = __tb_segment;
  __dpmi_int(0x21, &r);
  if(r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
 
  return r.x.cx;
}
