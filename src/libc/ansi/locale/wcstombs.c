/* Copyright (C) 2007 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2005 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

size_t
wcstombs(char *s, const wchar_t *wcs, size_t n)
{
  size_t i;
  for (i=0; (i<n) && wcs[i]; i++)
    s[i] = wcs[i];
  if (i<n)
    s[i] = 0;
  return i;
}
