/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

#include <malloc.h>
#include <unistd.h>

void *
valloc(size_t amt)
{
  static int page_size = -1;

  if (page_size == -1)
    page_size = getpagesize();

  return memalign(page_size, amt);
}
