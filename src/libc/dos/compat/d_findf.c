/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_FINDF.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <libc/stubs.h>
#include <libc/dosio.h>
#include <go32.h>
#include <errno.h>
#include <dpmi.h>
#include <string.h>
#include <dos.h>

unsigned int _dos_findfirst(const char *name, unsigned int attr, struct _find_t *result)
{
  __dpmi_regs r;

  _put_path(name);
  r.x.dx = (__tb & 15) + strlen(name) + 1;
  r.x.ds = __tb / 16;
  r.h.ah = 0x1A;
  __dpmi_int(0x21, &r);

  r.h.ah = 0x4E;
  r.x.dx = (__tb & 15);
  r.x.ds = __tb / 16;
  r.x.cx = attr;
  __dpmi_int(0x21, &r);
  if ( r.x.flags & 1 )
  {
    errno = __doserr_to_errno(r.x.ax);
    return r.x.ax;
  }
  dosmemget(__tb + strlen(name) + 1, sizeof(struct _find_t), result);
  return 0;
}
