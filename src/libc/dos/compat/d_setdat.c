/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_SETDAT.C.
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

unsigned int _dos_setdate(struct _dosdate_t *date)
{
  __dpmi_regs r;

  if (date == 0)
  {
    errno = EINVAL;
    return -1;
  }

  r.h.ah = 0x2B;
  r.x.cx = date->year;
  r.h.dh = date->month;
  r.h.dl = date->day;
  __dpmi_int(0x21, &r);
  if ( r.h.al )
  {
    errno = EINVAL;
    return 1;
  }
  return 0;
}
