/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <string.h>

#undef bcmp

int
bcmp(const void *ptr1, const void *ptr2, int length)
{
  if (ptr1 == ptr2)
    return 0;

  if (ptr1 == 0 || ptr2 == 0)
    return -1;

  while (length)
  {
    if (*((const char *)ptr1)++ != *((const char *)ptr2)++)
      return length;
    length--;
  }

  return 0;
}
