/* Copyright (c) 1995-98 Eli Zaretskii <eliz@is.elta.co.il> */

#include <dpmi.h>
#include <dos.h>

int
_is_cdrom_drive(int drive_num)
{
  __dpmi_regs r;

  r.x.ax = 0x150b;      /* CD-ROM Drive Check function */
  r.x.cx = drive_num - 1; /* 0 = A: */
  __dpmi_int(0x2f, &r);

  /* If MSCDEX installed, BX will hold ADADh; AX will be non-zero
     if this drive is supported by MSCDEX.  */
  if (r.x.bx == 0xadad && r.x.ax != 0)
    return 1;
  return 0;
}
