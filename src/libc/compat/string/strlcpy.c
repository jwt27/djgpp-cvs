/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <string.h>

size_t
strlcpy (char *dst, const char *src, size_t size)
{
  const size_t srclen = strlen(src);
  const size_t ret    = srclen;

  if (!size)
    return(ret); /* No space at all. */

  if (srclen < size) {
    /* Enough space - just copy the string. */
    strcpy(dst, src);
  } else {
    /* Truncate the string to fit. */
    memcpy(dst, src, size - 1);
    dst[size - 1] = '\0';
  }

  return(ret);
}
