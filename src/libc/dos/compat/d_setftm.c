/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_SETFTM.C.
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

unsigned int _dos_setftime(int handle, unsigned int date, unsigned int time)
{
  __dpmi_regs r;

  r.x.ax = 0x5701;
  r.x.bx = handle;
  r.x.cx = time;
  r.x.dx = date;
  __dpmi_int(0x21, &r);
  if ( r.x.flags & 1 )
  {
    errno = EBADF;
    return r.x.ax;
  }
  return 0;
}
