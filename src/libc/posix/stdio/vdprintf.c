/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
vdprintf(int fd, const char *fmt, va_list ap)
{
  char *string;
  int written;


  written = vasprintf(&string, fmt, ap);
  if (string)
  {
    written = write(fd, string, written);
    free(string);
  }

  return written;
}
