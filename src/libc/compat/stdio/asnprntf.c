/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2008 DJ Delorie, see COPYING.DJ for details */
#include <stdarg.h>
#include <stdio.h>

char *
asnprintf(char *str, size_t *np, const char *fmt, ...)
{
  va_list args;
  char *buffer;

  va_start(args, fmt);
  buffer = vasnprintf(str, np, fmt, args);
  va_end(args);

  return buffer;
}
