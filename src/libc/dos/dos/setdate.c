/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <dpmi.h>

void setdate( struct date *dateblk)
{
  __dpmi_regs regs;

  regs.h.ah = 0x2b;
  regs.x.cx = dateblk-> da_year;
  regs.h.dh = dateblk-> da_mon;
  regs.h.dl = dateblk-> da_day;
  __dpmi_int(0x21, &regs);
}
