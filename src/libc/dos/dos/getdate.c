/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <dpmi.h>

void getdate( struct date *dateblk)
{
  __dpmi_regs regs;

  regs.h.ah = 0x2a;
  __dpmi_int(0x21, &regs);
  dateblk-> da_year = regs.x.cx;
  dateblk-> da_mon = regs.h.dh;
  dateblk-> da_day = regs.h.dl;
}
