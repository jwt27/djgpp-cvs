/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <dpmi.h>

void
__dpmi_yield(void)
{
  __dpmi_regs r;

  r.x.ax = 0x1680;
  r.x.ss = r.x.sp = r.x.flags = 0;
  __dpmi_simulate_real_mode_interrupt (0x2f, &r);
  if (r.h.al == 0x80)
    errno = ENOSYS;
}
