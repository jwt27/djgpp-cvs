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

  _strbuf._flag = _IOWRT|_IOSTRG|_IONTERM;
  _strbuf._ptr = str;
  _strbuf._cnt = INT_MAX;

  va_start(args, fmt);
  len = _doprnt(fmt, args, &_strbuf);
  va_end(args);

  *_strbuf._ptr = 0;
  return len;
}
