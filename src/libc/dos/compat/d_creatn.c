/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_CREATN.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <libc/dosio.h>
#include <go32.h>
#include <dpmi.h>
#include <errno.h>
#include <dos.h>

unsigned int _dos_creatnew(const char *filename, unsigned int attr, int *handle)
{
  __dpmi_regs r;

  _put_path(filename);
  r.h.ah = 0x5B;
  r.x.cx = attr & 0xFFFF;
  r.x.dx = __tb & 15;
  r.x.ds = __tb / 16;
  __dpmi_int(0x21, &r);
  if ( r.x.flags & 1 )
  {
    errno  = __doserr_to_errno(r.x.ax);
    *handle = (unsigned)-1;
    return r.x.ax;
  }
  *handle = r.x.ax;
  return 0;
}
