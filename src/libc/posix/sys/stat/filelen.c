/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* This is file FILELEN.C */
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

long __filelength(int);

long
__filelength(int fhandle)
{
  __dpmi_regs    regs;
  unsigned short fpos_high, fpos_low;
  long           retval;

  /* Remember the current file position, so we can return there
     later.  */
  regs.x.ax = 0x4201;      /* set pointer from current position */
  regs.x.bx = fhandle;
  regs.x.cx = regs.x.dx = 0; /* move 0 bytes (i.e., stay put) */
  __dpmi_int(0x21, &regs);
  if (regs.x.flags & 1)
    {
      errno = __doserr_to_errno(regs.x.ax);
      return -1L;
    }
  fpos_high = regs.x.dx;   /* save current position */
  fpos_low  = regs.x.ax;

  regs.x.cx = regs.x.dx = 0;
  regs.x.ax = 0x4202;      /* set pointer 0 bytes from the end of file */
  __dpmi_int(0x21, &regs);
  if (regs.x.flags & 1)
    {
      errno = __doserr_to_errno(regs.x.ax);
      return -1L;
    }

  /* The absolute byte offset returned in DX:AX is the file size. */
  retval = ( (long)regs.x.dx << 16 ) + regs.x.ax;

  /* Leave things as we have found them. */
  regs.x.ax = 0x4200;      /* set pointer from the beginning of file */
  regs.x.cx = fpos_high;
  regs.x.dx = fpos_low;
  __dpmi_int(0x21, &regs);
  if (regs.x.flags & 1)
    {
      errno = __doserr_to_errno(regs.x.ax);
      return -1L;
    }

  return retval;
}
