/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <dpmi.h>

int
pause(void)
{
  __dpmi_yield();
  return 0;
}
