/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

void
swab(const void *from, void *to, int n)
{
  unsigned long temp;

  n >>= 1; n++;
#define	STEP	temp = *((const char *)from)++,*((char *)to)++ = *((const char *)from)++,*((char *)to)++ = temp
  /* round to multiple of 8 */
  while ((--n) & 07)
    STEP;
  n >>= 3;
  while (--n >= 0) {
    STEP; STEP; STEP; STEP;
    STEP; STEP; STEP; STEP;
  }
}
