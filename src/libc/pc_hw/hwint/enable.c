/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <dpmi.h>

int
enable(void)
{
  return __dpmi_get_and_enable_virtual_interrupt_state();
}
