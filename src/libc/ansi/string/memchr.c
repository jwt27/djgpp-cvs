/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <string.h>
#include <libc/unconst.h>

void *
memchr(const void *s, int c, size_t n)
{
  if (n)
  {
    const char *p = s;
    char cc = c;
    do {
      if (*p == cc)
	return unconst(p, void *);
      p++;
    } while (--n != 0);
  }
  return 0;
}
