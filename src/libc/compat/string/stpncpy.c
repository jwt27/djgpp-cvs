/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
#include <string.h>

char *
stpncpy(char *dst, const char *src, size_t n)
{
  if (!dst || !src)
    return 0;

  if (n != 0) {
    do {
      if ((*dst++ = *src++) == 0)
      {
	dst += n - 1;
	do {
	  *--dst = 0;
	} while (--n != 0);
	break;
      }
    } while (--n != 0);
  }
  return dst;
}
