/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

size_t
mbstowcs(wchar_t *wcs, const char *s, size_t n)
{
  size_t i;
  for (i=0; s[i] && (i+1<n); i++)
    wcs[i] = s[i];
  wcs[i] = 0;
  return i;
}
