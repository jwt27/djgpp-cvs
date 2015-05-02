/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdarg.h>
#include <libc/file.h>

int
vsscanf(const char *str, const char *fmt, va_list ap)
{
  FILE _strbuf;

  __stropenr(&_strbuf, str);
  return _doscan(&_strbuf, fmt, ap);
}
