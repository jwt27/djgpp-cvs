/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_GETDAT.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <dpmi.h>
#include <errno.h>
#include <dos.h>

void _dos_getdate(struct _dosdate_t *date)
{
  __dpmi_regs r;

  r.h.ah = 0x2A;
  __dpmi_int(0x21, &r);
  date->year      = (unsigned short)r.x.cx;
  date->month     = r.h.dh;
  date->day       = r.h.dl;
  date->dayofweek = r.h.al;
}
