/* Copyright (C) 2007 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2005 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

size_t
mbstowcs(wchar_t *wcs, const char *s, size_t n)
{
  size_t i;
  for (i=0; (i<n) && s[i]; i++)
    wcs[i] = s[i];
  if (i<n)
    wcs[i] = 0;
  return i;
}
