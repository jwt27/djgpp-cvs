/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <io.h>
#include <sys/fsext.h>

#include <libc/dosio.h>

int
_close(int handle)
{
  __dpmi_regs r;

  __FSEXT_Function *func = __FSEXT_get_function(handle);
  if (func)
  {
    int rv;
    if (func(__FSEXT_close, &rv, &handle))
      return rv;
  }

  r.h.ah = 0x3e;
  r.x.bx = handle;
  __dpmi_int(0x21, &r);
  if (r.x.flags & 1)
  {
    errno = EBADF;
    return -1;
  }
  return 0;
}
