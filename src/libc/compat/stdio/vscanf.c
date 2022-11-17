/* Copyright (C) 2018 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2017 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdarg.h>

int
vscanf(const char *fmt, va_list ap)
{
  return _doscan(stdin, fmt, ap);
}

int
vfscanf(FILE *stream, const char *fmt, va_list ap)
{
  return _doscan(stream, fmt, ap);
}
