/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_SETFA.C.
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

unsigned int _dos_setfileattr(const char *filename, unsigned int attr)
{
  __dpmi_regs r;

  _put_path(filename);
  r.x.ax = 0x4301;
  r.x.cx = attr & 0xFFFF;
  r.x.dx = __tb & 15;
  r.x.ds = __tb / 16;
  __dpmi_int(0x21, &r);
  if ( r.x.flags & 1 )
  {
    errno = __doserr_to_errno(r.x.ax);
    return r.x.ax;
  }
  return 0;
}
