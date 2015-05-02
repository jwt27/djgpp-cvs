/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <string.h>

#undef bcmp

int
bcmp(const void *vptr1, const void *vptr2, int length)
{
  const char *ptr1 = vptr1;
  const char *ptr2 = vptr2; 
  if (ptr1 == ptr2)
    return 0;

  if (ptr1 == 0 || ptr2 == 0)
    return -1;

  while (length)
  {
    if (*ptr1++ != *ptr2++)
      return length;
    length--;
  }

  return 0;
}
