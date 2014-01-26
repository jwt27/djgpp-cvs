/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2008 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <libc/file.h>

char *
vasnprintf(char *str, size_t *np, const char *fmt, va_list argsp)
{
  FILE _strbuf;
  char *buffer = str;
  int len = *np;


  /*  _cnt is an int in the FILE structure. To prevent wrap-around,
   *  we limit n to between 0 and INT_MAX inclusively.  */
  if (len > INT_MAX)
  {
    errno = EFBIG;
    return NULL;
  }

  memset(&_strbuf, 0, sizeof(_strbuf));
  if (buffer && len > 0)
    len--;
  else
  {
    /*  Just query how much space is needed.  */
    __stropenw(&_strbuf, NULL, 0);
    len = _doprnt(fmt, argsp, &_strbuf);
    if (len == EOF)
      return NULL;
    buffer = malloc(len + 1);
    if (!buffer)
      return NULL;
  }

  __stropenw(&_strbuf, buffer, len);
  len = _doprnt(fmt, argsp, &_strbuf);
  if (len == EOF)
  {
    free(buffer);
    buffer = NULL;
  }
  else if (len > 0)
  {
    *np = len;
    __strclosew(&_strbuf);  /* Ensure nul termination */
  }

  return buffer;
}
