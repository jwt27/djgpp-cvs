/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_EXTERR.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <dos.h>
#include <dpmi.h>

int _dosexterr(struct _DOSERROR *p_error)
{
  __dpmi_regs r;

  r.h.ah = 0x59;
  r.x.bx = 0;
  __dpmi_int(0x21, &r);
  p_error->exterror = (int)r.x.ax;
  p_error->class    = r.h.bh;
  p_error->action   = r.h.bl;
  p_error->locus    = r.h.ch;
  return (int)r.x.ax;
}
