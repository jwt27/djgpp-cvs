/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <fcntl.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <io.h>
#include <dos.h>
#include <libc/dosio.h>
#include <sys/fsext.h>

int
_creatnew(const char* filename, int attrib, int flags)
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
  r.x.bx =
    0x2002 | (flags & 0xfff0);	/* r/w, no Int 24h, use caller-defined flags */
  r.x.dx = 0x0010;		/* Create, fail if exists */
  r.x.si = __tb_offset;
  if(use_lfn)
    r.x.ax = 0x716c;
  else
  {
    if (_osmajor == 0)
      _get_dos_version (0);
    if (_osmajor > 3)
      r.x.ax = 0x6c00;
    else
    {
      r.h.ah = 0x5b;
      r.x.bx = 0;		/* lose support for fancy flags in DOS 3.x */
      r.x.dx = __tb_offset;
      r.x.si = 0;
    }
  }
  r.x.cx = attrib & 0xffff;
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
