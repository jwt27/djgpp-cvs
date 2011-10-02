/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
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

  if (_USE_LFN)
  {
    r.x.flags = 1;			/* Always set CF before calling a 0x71NN function. */
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
  if ((r.x.flags & 1) || (r.x.ax == 0x7100))
  {
    /*  Never assume that the complete LFN API is implemented,
        so check that AX != 0x7100.  E.G.: MSDOS 6.22 and DOSLFN 0.40.
        If not supported fail.  */
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
 
  return r.x.cx;
}
