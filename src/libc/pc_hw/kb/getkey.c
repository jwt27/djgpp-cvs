/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <pc.h>
#include <dpmi.h>

int
getkey(void)
{
  __dpmi_regs r;
  r.h.ah = 0x10;
  __dpmi_int(0x16, &r);

  if (r.h.al == 0x00 || (r.h.al == 0xe0 && r.h.ah != 0))
    return 0x0100 | r.h.ah;
  return r.h.al;
}
