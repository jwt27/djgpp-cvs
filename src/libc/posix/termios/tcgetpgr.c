/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */

#include <libc/stubs.h>
#include <unistd.h>
#include <errno.h>

pid_t
tcgetpgrp (int fd)
{
  int e = errno;

  if (isatty (fd))
    {
      errno = e;
      return getpgrp ();
    }
  if (!errno)
    errno = ENOTTY;
  return -1;
}
