/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <pc.h>
#include <libc/farptrgs.h>
#include <dpmi.h>
#include <go32.h>


int
kbhit(void)
{
  __dpmi_regs r;

  if (_farpeekw(_dos_ds, 0x41a) == _farpeekw(_dos_ds, 0x41c))
    return 0;

  r.h.ah = 0x11;
  __dpmi_int(0x16, &r);
  if (r.x.flags & 0x40) /* Z */
    return 0;
  return 1;
}
