/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_GETTIM.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <dpmi.h>
#include <dos.h>

void _dos_gettime(struct _dostime_t *time)
{
  __dpmi_regs r;

  r.h.ah = 0x2C;
  __dpmi_int(0x21, &r);
  time->hour    = r.h.ch;
  time->minute  = r.h.cl;
  time->second  = r.h.dh;
  time->hsecond = r.h.dl;
}
