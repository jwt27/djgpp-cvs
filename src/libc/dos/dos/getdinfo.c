/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* This is file GETDINFO.C */
/*
 * Get device info word by calling IOCTL Function 0.
 *
 * Copyright (c) 1994 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely so long as this copyright notice is
 * left intact.  There is no warranty on this software.
 *
 */

#include <errno.h>
#include <dpmi.h>
#include <libc/dosio.h>

short _get_dev_info(int);

short
_get_dev_info(int fhandle)
{
  __dpmi_regs regs;

  regs.x.ax = 0x4400;
  regs.x.bx = fhandle & 0x0000ffff;
  __dpmi_int(0x21, &regs);

  if (regs.x.flags & 1)     /* error */
    {
      errno = __doserr_to_errno(regs.x.ax);
      return -1;
    }
  else
    return regs.x.dx;
}
