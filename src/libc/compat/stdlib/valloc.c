/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

#include <stdlib.h>
#include <unistd.h>

void *
valloc (size_t amt)
{
  static int page_size = -1;

  if (page_size == -1)
    page_size = getpagesize();

  return memalign (amt, page_size);
}
