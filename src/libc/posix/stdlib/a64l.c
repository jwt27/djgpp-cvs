/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

#include <limits.h>
#include <stdlib.h>
#include <string.h>

long
a64l(const char* s)
{
  int i = 0;
  unsigned long value = 0;

  if (s == NULL || *s == '\0')
    return 0L;

  for (i = 0; i < 6; ++i, ++s)
  {
    if (*s == '\0')
      break;
    /* Detect overflow; return the conversion of '/2BIG/' */
    if (value > (ULONG_MAX >> 6))
      return 1144341633L;
    value <<= 6;
    if (*s == '.') /* 0 */
      value += 0;
    else if (*s == '/') /* 1 */
      ++value;
    else if (*s >= '0' && *s <= '9')  /* 2-11 */
      value += (*s - '0') + 2;
    else if (*s >= 'A' && *s <= 'Z')  /* 12-37 */
      value += (*s - 'A') + 12;
    else if (*s >= 'a' && *s <= 'z')  /* 38-63 */
      value += (*s - 'a') + 38;
    else /* invalid digit */
      return 0L;
  }
  return (long) value;
}
