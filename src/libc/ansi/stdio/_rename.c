/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <libc/dosio.h>
 
int _rename(const char *old, const char *new)
{
  __dpmi_regs r;
  int olen    = strlen(old) + 1;
  int i;

  r.x.dx = __tb_offset;
  r.x.di = __tb_offset + olen;
  r.x.ds = r.x.es = __tb_segment;

  for (i=0; i<2; i++)
  {
    if(_USE_LFN)
      r.x.ax = 0x7156;
    else
      r.h.ah = 0x56;
    _put_path2(new, olen);
    _put_path(old);
    __dpmi_int(0x21, &r);
    if(r.x.flags & 1)
    {
      if (r.x.ax == 5 && i == 0) /* access denied */
	remove(new);		 /* and try again */
      else
      {
	errno = __doserr_to_errno(r.x.ax);
	return -1;
      }
    }
    else
      break;
  }
  return 0;
}

