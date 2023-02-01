/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>

extern int __brk(void *);
extern void *___sbrk(int delta);

int
brk(void *_heaptop)
{
  if (__brk(_heaptop) == -1)
    return -1;
  return 0;
}

void *sbrk(int delta)
{
  return ___sbrk(delta);
}
