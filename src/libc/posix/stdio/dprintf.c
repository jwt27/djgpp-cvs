/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdarg.h>

int
dprintf(int fd, const char *fmt, ...)
{
  va_list arg_list;
  int written;


  va_start(arg_list, fmt);
  written = vdprintf(fd, fmt, arg_list);
  va_end(arg_list);

  return written;
}
