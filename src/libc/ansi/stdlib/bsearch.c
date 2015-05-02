/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <libc/unconst.h>

void *
bsearch(const void *key, const void *base, size_t nelem,
	size_t size, int (*cmp)(const void *ck, const void *ce))
{
  int lim, cmpval;
  const void *p;

  for (lim = nelem; lim != 0; lim >>= 1)
  {
    p = (const char *)base + (lim >> 1) * size;
    cmpval = (*cmp)(key, p);
    if (cmpval == 0)
      return unconst(p, void *);
    if (cmpval > 0)
    {				/* key > p: move right */
      base = (const char *)p + size;
      lim--;
    } /* else move left */
  }
  return NULL;
}
