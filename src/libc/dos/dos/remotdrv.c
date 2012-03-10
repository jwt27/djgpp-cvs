/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * This is file REMOTDRV.C
 *
 * Copyright (c) 1994, 1995 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely so long as this copyright notice is
 * left intact.  There is no warranty on this software.
 *
 */
 
#include <errno.h>
#include <dos.h>
#include <dpmi.h>
#include <libc/dosio.h>
 
int _is_remote_drive(int);
 
/* Return 1 if the named DRIVE is remote (i.e., networked) drive,
   0 if not.  Return -1 if the call failed.
   Call with drive NUMBER, not letter, i.e. 0 = 'A', 1 = 'B', etc.
   The drive number passed as argument to the function MUST be zero based!!  */
 
int
_is_remote_drive(int drv_no)
{
  __dpmi_regs regs;
 
  if (_get_dos_version(1) < 0x030a)
  {
    /* DOS versions before 3.1 don't support INT 21H/AX=4409H, but
       they don't support network, either.  So we just verify that
       this is a valid drive, and if so, return 0 (i.e., local
       drive).  */
    regs.h.ah = 0x1c;
    regs.h.dl = drv_no + 1;	/* 0 = default, 1 = 'A', etc.  */
    __dpmi_int(0x21, &regs);
    if (regs.h.al == 0xff)
    {
      /* INT 21H/AH=1CH doesn't return an error code, so fake one.  */
      errno = ENODEV;
      return -1;
    }
    return 0;
  }
 
  /* INT 21H/AX=4409H returns DX with 12th bit set, if drive is remote. */
  regs.x.ax = 0x4409;		/* IOCTL, Subfunction 09h */
  regs.h.bl = drv_no + 1;	/* 0 = default, 1 = 'A', etc.  */
  __dpmi_int(0x21, &regs);
 
  if (regs.x.flags & 1)
  {
    errno = __doserr_to_errno(regs.x.ax);
    return -1;
  }
  if (regs.x.dx & 0x1000)
    return 1;
  return 0;
}
