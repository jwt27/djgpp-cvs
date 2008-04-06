/* Copyright (C) 2008 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <libc/file.h>

int
vasnprintf(char **strp, size_t n, const char *fmt, va_list argsp)
{
  FILE _strbuf;
  int len;


  /*  _cnt is an int in the FILE structure. To prevent wrap-around,
   *  we limit n to between 0 and INT_MAX inclusively.  */
  if (n > INT_MAX)
  {
    errno = EFBIG;
    return EOF;
  }

  /*  Just query how much space is needed.  */
  memset(&_strbuf, 0, sizeof(_strbuf));
  __stropenw(&_strbuf, NULL, 0);
  len = _doprnt(fmt, argsp, &_strbuf);

  *strp = NULL;
  if (n > 0)
  {
    if (len != EOF)
    {
      len++;
      if ((size_t)len > n)
        len = n;
      *strp = malloc(len);
      if (*strp)
      {
        __stropenw(&_strbuf, *strp, len - 1);
        len = _doprnt(fmt, argsp, &_strbuf);
        __strclosew(&_strbuf);
        if (len == EOF)
        {
          free(*strp);
          *strp = NULL;
        }
      }
    }
    else
      len = EOF;
  }

  return len;
}
