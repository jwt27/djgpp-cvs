/* Copyright (c) 1995-98 Eli Zaretskii <eliz@is.elta.co.il> */

#include <dpmi.h>
#include <dos.h>

int
_media_type(int drive_num)
{
  __dpmi_regs r;

  r.x.ax = 0x4408;
  r.h.bl = drive_num;
  __dpmi_int(0x21, &r);

  if (r.x.flags & 1)
    return -1;
  return r.x.ax;    /* returns 1 for fixed disks, 0 for removable */
}
