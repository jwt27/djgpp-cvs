/* Copyright (C) 2008 DJ Delorie, see COPYING.DJ for details */
#include <stdarg.h>
#include <stdio.h>

int
asprintf(char **strp, const char *fmt, ...)
{
  va_list args;
  int len;

  va_start(args, fmt);
  len = vasprintf(strp, fmt, args);
  va_end(args);

  return len;
}
