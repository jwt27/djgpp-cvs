/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdarg.h>
#include <stdio.h>
#include <libc/file.h>

int
printf(const char *fmt, ...)
{
  va_list args;
  int len;

  va_start(args, fmt);
  len = _doprnt(fmt, args, stdout);
  va_end(args);

  /* People were confused when printf() didn't flush stdout,
     so we'll do it to reduce confusion */
  if (stdout->_flag & _IOLBF)
    fflush(stdout);

  return ferror(stdout) ? EOF : len;
}
