/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

extern unsigned __djgpp_stack_limit;

/* make return value signed, just in case we already overflowed the stack */
int stackavail(void)
{
  unsigned sp;
  __asm__ __volatile__ ("movl %%esp,%k0\n" : "=r" (sp) : );
  return (int) (sp - __djgpp_stack_limit);
}


