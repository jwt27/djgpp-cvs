/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * BIOSKEY.C.
 *
 * Modified by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <bios.h>
#include <dpmi.h>

int
bioskey(int cmd)
{
  __dpmi_regs r;
  r.h.ah = cmd;
  __dpmi_int(0x16, &r);
  switch ( cmd )
  {
  case 0x00:
  case 0x10:
    return r.x.ax & 0xffff;
  case 0x01:
  case 0x11:
    if ((r.x.flags & 0x40 ) == 0x40 )
      return 0;
    else
      /* CTRL-BREAK checking */
      return (!r.x.ax) ? -1 : (r.x.ax & 0xffff);
  case 0x02:
    return r.h.al;
  case 0x12:
    return r.x.ax & 0xffff;
  }
  return 0;
}
