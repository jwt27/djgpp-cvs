/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <io.h>
#include <sys/fsext.h>

#include <libc/dosio.h>

int
_write(int handle, const void* buffer, size_t count)
{
  size_t j, i;
  int nput;
  unsigned long tbsize;
  __dpmi_regs r;

  __FSEXT_Function *func = __FSEXT_get_function(handle);
  if (func)
  {
    int rv;
    if (func(__FSEXT_write, &rv, &handle))
      return rv;
  }

  tbsize = _go32_info_block.size_of_transfer_buffer;
  nput = 0;
  do {
    j = (count <= tbsize) ? count : tbsize;
    if (j)
      dosmemput(buffer, j, __tb);
    r.x.ax = 0x4000;
    r.x.bx = handle;
    r.x.cx = j;
    r.x.dx = __tb & 15;
    r.x.ds = __tb / 16;
    __dpmi_int(0x21, &r);
    if (r.x.flags & 1)
    {
      errno = __doserr_to_errno(r.x.ax);
      return -1;
    }
    i = r.x.ax;
    count -= i;
    buffer = (void *)((int)buffer + i);
    nput += i;
  } while(count && (i == j));

  if (count && nput == 0)
  {
    errno = ENOSPC;
    return -1;
  }

  return nput;
}
