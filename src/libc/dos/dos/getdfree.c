/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <dpmi.h>
#include <errno.h>

void
getdfree(unsigned char drive, struct dfree *dtable)
{
  __dpmi_regs regs;

  regs.h.ah = 0x36;
  regs.h.dl = drive;
  __dpmi_int(0x21, &regs);
  if (regs.x.ax == 0xffff)
  {
    errno = ENODEV;
    dtable->df_sclus = 0;
    dtable->df_avail = 0;
    dtable->df_bsec = 0;
    dtable->df_total = 0;
    return;
  }
  dtable->df_sclus = regs.x.ax;
  dtable->df_avail = regs.x.bx;
  dtable->df_bsec = regs.x.cx;
  dtable->df_total = regs.x.dx;
}
