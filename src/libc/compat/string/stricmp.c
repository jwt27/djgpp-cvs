/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>
#include <ctype.h>

int
stricmp(const char *s1, const char *s2)
{
  while (tolower((unsigned char)*s1) == tolower((unsigned char)*s2))
  {
    if (*s1 == 0)
      return 0;
    s1++;
    s2++;
  }
  return (int)tolower((unsigned char)*s1) - (int)tolower((unsigned char)*s2);
}
