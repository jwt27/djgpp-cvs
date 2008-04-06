/* Copyright (C) 2008 DJ Delorie, see COPYING.DJ for details */
#include <stdarg.h>
#include <stdio.h>

int
asnprintf(char **strp, size_t n, const char *fmt, ...)
{
  va_list args;
  int len;

  va_start(args, fmt);
  len = vasnprintf(strp, n, fmt, args);
  va_end(args);

  return len;
}
