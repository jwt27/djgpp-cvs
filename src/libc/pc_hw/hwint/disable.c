/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <dpmi.h>

int
disable(void)
{
  return __dpmi_get_and_disable_virtual_interrupt_state();
}
