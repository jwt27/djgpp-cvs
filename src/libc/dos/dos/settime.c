/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <dpmi.h>

void settime( struct time *tp)
{
  __dpmi_regs regs;
  regs.h.ah = 0x2d;
  regs.h.ch = tp->ti_hour;
  regs.h.cl = tp->ti_min;
  regs.h.dh = tp->ti_sec;
  regs.h.dl = tp->ti_hund;
  __dpmi_int(0x21, &regs);
}
