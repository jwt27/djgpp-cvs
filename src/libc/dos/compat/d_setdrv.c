/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_SETDRV.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <dos.h>
#include <dpmi.h>

void _dos_setdrive(unsigned int drive, unsigned int *p_drives)
{
  __dpmi_regs r;

  r.h.ah = 0x0E;
  r.h.dl = drive - 1;
  __dpmi_int(0x21, &r);
  *p_drives = (unsigned int)r.h.al;
}
