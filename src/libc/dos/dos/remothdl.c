/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* This is file REMOTHDL.C */
/*
 * Copyright (c) 1994 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely so long as this copyright notice is
 * left intact.  There is no warranty on this software.
 *
 */

#include <errno.h>
#include <dpmi.h>
#include <libc/dosio.h>

int _is_remote_handle(int);

int
_is_remote_handle(int fhandle)
{
  __dpmi_regs regs;

  regs.x.ax = 0x440a;
  regs.x.bx = fhandle & 0x0000ffff;
  __dpmi_int(0x21, &regs);

  if (regs.x.flags & 1)     /* error */
    {
      errno = __doserr_to_errno(regs.x.ax);
      return -1;
    }
  else
    /* IOCTL Function 0Ah returns 15th bit set for remote handles. */
    return regs.x.dx & 0x8000;
}
