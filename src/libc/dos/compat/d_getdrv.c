/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_GETDRV.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <dos.h>
#include <dpmi.h>

void _dos_getdrive(unsigned int *p_drive)
{
  __dpmi_regs r;

  r.h.ah = 0x19;
  __dpmi_int(0x21, &r);
  *p_drive = (unsigned int)r.h.al + 1;
}
