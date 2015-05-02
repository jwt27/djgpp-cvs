/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include <libc/file.h>

int
sprintf(char *str, const char *fmt, ...)
{
  va_list args;
  FILE _strbuf;
  int len;

  __stropenw(&_strbuf, str, INT_MAX);
  va_start(args, fmt);
  len = _doprnt(fmt, args, &_strbuf);
  va_end(args);
  __strclosew(&_strbuf);
  return len;
}
