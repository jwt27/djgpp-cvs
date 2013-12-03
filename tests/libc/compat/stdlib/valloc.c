/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
  int pagesize = getpagesize();
  char *p = valloc(pagesize);
  int i = (size_t)p;


  if (i & (pagesize - 1))
  {
    printf("Alignment problem: valloc returns %p\n", p);
    return 1;
  }
  else
    printf("valloc check passed\n");

  return 0;
}
