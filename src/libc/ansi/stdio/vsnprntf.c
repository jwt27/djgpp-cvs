/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <libc/file.h>

int
vsnprintf(char *str, size_t n, const char *fmt, va_list ap)
{
  FILE _strbuf;
  int len;

  /* _cnt is an int in the FILE structure. To prevent wrap-around, we limit
   * n to between 0 and INT_MAX inclusively. */
  if (n > INT_MAX)
  {
    errno = EFBIG;
    return -1;
  }

  memset(&_strbuf, 0, sizeof(_strbuf));
  _strbuf._flag = _IOWRT | _IOSTRG | _IONTERM;  

  /* If n == 0, just querying how much space is needed. */
  if (n > 0)
  {
    _strbuf._cnt = n - 1;
    _strbuf._ptr = str;
  }
  else
  {
    _strbuf._cnt = 0;
    _strbuf._ptr = NULL;
  }

  len = _doprnt(fmt, ap, &_strbuf);

  /* Ensure nul termination */
  if (n > 0)
    *_strbuf._ptr = 0;

  return len;
}
