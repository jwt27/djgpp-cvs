/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <fcntl.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <io.h>
#include <libc/dosio.h>
#include <sys/fsext.h>

int
_creat(const char* filename, int attrib)
{
  __dpmi_regs r;
  int rv;
  unsigned use_lfn = _USE_LFN;

  if (filename == 0)
  {
    errno = EINVAL;
    return -1;
  }

  if (__FSEXT_call_open_handlers(__FSEXT_creat, &rv, &filename))
    return rv;

  _put_path(filename);
  if(use_lfn) {
    r.x.ax = 0x716c;
    r.x.bx = 0x0002;		/* open r/w */
    r.x.dx = 0x0012;		/* Create, truncate if exists */
    r.x.si = __tb_offset;
  } else {
    r.h.ah = 0x3c;
    r.x.dx = __tb_offset;
  }
  r.x.cx = attrib;
  r.x.ds = __tb_segment;
  __dpmi_int(0x21, &r);
  if(r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  __file_handle_set(r.x.ax, O_BINARY);
  return r.x.ax;
}
