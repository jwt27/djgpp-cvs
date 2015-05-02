/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
#include <stdarg.h>
#include <stdio.h>

int
snprintf(char *str, size_t n, const char *fmt, ...)
{
  va_list ap;
  int len;

  va_start(ap, fmt);
  len = vsnprintf(str, n, fmt, ap);
  va_end(ap);

  return len;
}
