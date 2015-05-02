/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <string.h>

void *
calloc(size_t nelem, size_t elsize)
{
  register size_t size = 	nelem * elsize;
  void *rv = malloc(size);
  if (rv)
    memset(rv, 0, size);
  return rv;
}
