/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <libc/file.h>

int
printf(const char *fmt, ...)
{
  int len;

  len = _doprnt(fmt, (&fmt)+1, stdout);
  return ferror(stdout) ? EOF : len;
}
