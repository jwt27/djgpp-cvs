/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_GETFTM.C.
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

unsigned int _dos_getftime(int handle, unsigned int *p_date, unsigned int *p_time)
{
  __dpmi_regs r;

  if (p_date == 0 || p_time == 0)
  {
    errno = EINVAL;
    return -1;
  }

  r.x.ax = 0x5700;
  r.x.bx = handle;
  __dpmi_int(0x21, &r);
  if ( r.x.flags & 1 )
  {
    errno = EBADF;
    return r.x.ax;
  }
  *p_time = r.x.cx;
  *p_date = r.x.dx;
  return 0;
}
