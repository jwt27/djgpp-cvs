/* Copyright (C) 2018 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2017 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdarg.h>
#include <libc/file.h>
#include <libc/doprsc.h>

int
scanf(const char *fmt, ...)
{
  int r;
  va_list a=0;
  va_start(a, fmt);
  r = _doscan(stdin, fmt, a);
  va_end(a);
  return r;
}
