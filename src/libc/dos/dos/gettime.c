/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <dpmi.h>

void gettime( struct time *tp)
{
  __dpmi_regs regs;

  regs.h.ah = 0x2c;
  __dpmi_int(0x21, &regs);
  tp->ti_hour = regs.h.ch;
  tp->ti_min = regs.h.cl;
  tp->ti_sec = regs.h.dh;
  tp->ti_hund = regs.h.dl;
}
