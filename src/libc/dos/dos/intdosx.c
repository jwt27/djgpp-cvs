/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>

#undef intdosx

int
intdosx(union REGS *in, union REGS *out, struct SREGS *seg)
{
  return int86x(0x21, in, out, seg);
}
