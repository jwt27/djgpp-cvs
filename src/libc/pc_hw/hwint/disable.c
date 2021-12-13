/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <dpmi.h>

int
disable(void)
{
  /* return __dpmi_get_and_disable_virtual_interrupt_state(); */
  long rv;
  asm volatile ("pushf; pop %0" : "=g" (rv));
  rv = (rv>>9) & 1;
  asm("cli");
  return rv;
}
