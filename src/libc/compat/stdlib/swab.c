/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

void
swab(const void *vfrom, void *vto, int n)
{
  unsigned long temp;
  const char *from=vfrom;
  char *to=vto;

  n >>= 1; n++;
#define	STEP	temp = *from++,*to++ = *from++,*to++ = temp
  /* round to multiple of 8 */
  while ((--n) & 07)
    STEP;
  n >>= 3;
  while (--n >= 0) {
    STEP; STEP; STEP; STEP;
    STEP; STEP; STEP; STEP;
  }
}
