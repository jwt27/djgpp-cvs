/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
#include <string.h>
#include <ctype.h>

int
memicmp(const void *s1, const void *s2, size_t n)
{
  if (n != 0)
  {
    const unsigned char *p1 = s1, *p2 = s2;

    do {
      if (*p1 != *p2)
      {
        int c = toupper((unsigned char)*p1) - toupper((unsigned char)*p2);
	if (c)
	  return c;
      }
      p1++; p2++;
    } while (--n != 0);
  }
  return 0;
}
