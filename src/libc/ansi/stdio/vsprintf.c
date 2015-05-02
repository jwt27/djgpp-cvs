/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <libc/file.h>

int
vsprintf(char *str, const char *fmt, va_list ap)
{
  FILE f;
  int len;

  __stropenw(&f, str, INT_MAX);
  len = _doprnt(fmt, ap, &f);
  __strclosew(&f);
  return len;
}
