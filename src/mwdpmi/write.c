/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */
#include "code32.h"

ssize_t
write (int handle, const void *buf, int count)
{
  int thiscount, buffer_size;
  void *low_buffer;
  int sent = 0;
  __dpmi_regs regs;

  if (transfer_buffer_seg)
    {
      buffer_size = PAGE_SIZE;
      regs.x.ds = transfer_buffer_seg;
      regs.x.dx = 0;
    }
  else
    {
      /* Use a small part of the server stack.  */
      buffer_size = 0x10;
      regs.x.ds = code_seg;
      regs.x.dx = (int)&server_stack_start;
    }

  regs.x.bx = handle;
  low_buffer = LINEAR_TO_PTR ((regs.x.ds << 4) + regs.x.dx);
  while (count > 0)
    {
      thiscount = (count > buffer_size) ? buffer_size : count;
      memcpy (low_buffer, buf, thiscount);
      regs.h.ah = DOS_WRITE_FILE;
      regs.x.cx = thiscount;
      server_int (INT_DOS, &regs);

      buf += thiscount;
      count -= thiscount;
      sent += thiscount;
    }
  return sent;
}
/* ---------------------------------------------------------------------- */
