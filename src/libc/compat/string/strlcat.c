/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <string.h>

size_t
strlcat (char *dst, const char *src, size_t size)
{
  const size_t srclen = strlen(src);
  size_t       dstlen = 0;
  size_t       i;

  /* Find the length of dst. Don't go past the size'th element. */
  for (i = 0; i < size; i++) { if (dst[i] == '\0') break; }

  if (i == size)
    /* dst is not nul terminated. As per OpenBSD and NetBSD, return
     * the sum of the buffer length and srclen. */
    return(size + srclen);
  else
    dstlen = i;

  if ((dstlen + srclen) < size) {
    /* Enough space - just copy the string. */
    strcpy(dst + dstlen, src);
  } else {
    /* Truncate the string to fit. */
    memcpy(dst + dstlen, src, size - dstlen - 1);
    dst[size - 1] = '\0';
  }

  return(dstlen + srclen);
}
