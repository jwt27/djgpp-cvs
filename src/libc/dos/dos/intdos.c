/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>

#undef intdos

int
intdos(union REGS *in, union REGS *out)
{
  return int86(0x21, in, out);
}
