/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <io.h>
#include <stdio.h>
#include <errno.h>
#include <dpmi.h>
#include <go32.h>
#include <libc/dosio.h>
 
int
remove(const char *fn)
{
  __dpmi_regs r;
  unsigned attr;
  int directory_p;
 
  /* Get the file attribute byte.  */
  attr = _chmod(fn, 0);
  directory_p = attr & 0x10;
 
  /* Now, make the file writable.  We must reset Vol, Dir, Sys and Hidden bits 
     in addition to the Read-Only bit, or else 214301 will fail.  */
  _chmod(fn, 1, attr & 0xffe0);

  /* Now delete it.  Note, _chmod leave dir name in tranfer buffer. */
  if (directory_p)
    r.h.ah = 0x3a;		/* DOS Remove Directory function */
  else
    r.h.ah = 0x41;		/* DOS Remove File function */
  if(_USE_LFN) {
    r.h.al = r.h.ah;
    r.h.ah = 0x71;
    r.x.si = 0;			/* No Wildcards */
  }
  r.x.dx = __tb_offset;
  r.x.ds = __tb_segment;
  __dpmi_int(0x21, &r);
  if(r.x.flags & 1)
  {
    /* We failed.  Leave the things as we've found them.  */
    int e = __doserr_to_errno(r.x.ax);
 
    _chmod(fn, 1, attr & 0xffe7);
    errno = e;
    return -1;
  }
  return 0;
}
