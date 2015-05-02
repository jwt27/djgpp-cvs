/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_GETFA.C.
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

unsigned int _dos_getfileattr(const char *filename, unsigned int *p_attr)
{
  __dpmi_regs r;

  if (filename == 0 || p_attr == 0)
  {
    errno = EINVAL;
    return -1;
  }

  _put_path(filename);
  r.x.ax = 0x4300;
  r.x.dx = __tb & 15;
  r.x.ds = __tb / 16;
  __dpmi_int(0x21, &r);
  if ( r.x.flags & 1 )
  {
    errno = __doserr_to_errno(r.x.ax);
    return r.x.ax;
  }
  *p_attr = r.x.cx;
  return 0;
}
