/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_CLOSE.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <libc/dosio.h>
#include <dpmi.h>
#include <errno.h>
#include <dos.h>

unsigned int _dos_close(int handle)
{
  __dpmi_regs r;

  r.h.ah = 0x3E;
  r.x.bx = handle;
  __dpmi_int(0x21, &r);
  if ( r.x.flags & 1 )
  {
    errno  = __doserr_to_errno(r.x.ax);
    return r.x.ax;
  }
  return 0;
}
