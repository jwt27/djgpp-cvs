/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_SETTIM.C.
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

unsigned int _dos_settime(struct _dostime_t *time)
{
  __dpmi_regs r;

  if (time == 0)
  {
    errno = EINVAL;
    return -1;
  }

  r.h.ah = 0x2D;
  r.h.ch = time->hour;
  r.h.cl = time->minute;
  r.h.dh = time->second;
  r.h.dl = time->hsecond;
  __dpmi_int(0x21, &r);
  if ( r.h.al )
  {
    errno = EINVAL;
    return 1;
  }
  return 0;
}
