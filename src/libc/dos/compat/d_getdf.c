/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_GETDF.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <dos.h>
#include <dpmi.h>
#include <errno.h>

unsigned int _dos_getdiskfree(unsigned int drive, struct _diskfree_t *diskspace)
{
  __dpmi_regs r;

  r.h.ah = 0x36;
  r.h.dl = drive;
  __dpmi_int(0x21, &r);
  if ( r.x.ax == 0xFFFF )
  {
    diskspace->sectors_per_cluster =
    diskspace->avail_clusters      =
    diskspace->bytes_per_sector    =
    diskspace->total_clusters      = 0;
    errno = EINVAL;
    return 1;
  }
  diskspace->sectors_per_cluster = r.x.ax;
  diskspace->avail_clusters      = r.x.bx;
  diskspace->bytes_per_sector    = r.x.cx;
  diskspace->total_clusters      = r.x.dx;
  return 0;
}
