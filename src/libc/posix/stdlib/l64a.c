/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

#include <limits.h>
#include <stdlib.h>
#include <string.h>

char*
l64a(long _value)
{
  static char radix64[7] = { "" };
  char *s = radix64 + 5;
  unsigned long value = (unsigned long) _value;

  memset (radix64, 0, sizeof radix64);

  if (value == 0)
    return radix64;

  while (value && s >= radix64)
  {
    int digit = value & 0x3f;
    value >>= 6;

    if (digit == 0)
      *s-- = '.';
    else if (digit == 1)
      *s-- = '/';
    else if (digit < 12)
      *s-- = '0' + (digit - 2);
    else if (digit < 38)
      *s-- = 'A' + (digit - 12);
    else
      *s-- = 'a' + (digit - 38);
  }
  return ++s;
}
