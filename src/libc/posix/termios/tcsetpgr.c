/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */

#include <libc/stubs.h>
#include <unistd.h>
#include <errno.h>

int
tcsetpgrp (int fd, pid_t pgid)
{
  int e = errno;
  pid_t pgrp =  getpgrp ();

  if (isatty (fd) && pgid == pgrp)
    {
      errno = e;
      return 0;
    }
  if (!errno)
    errno = pgid == pgrp ? ENOTTY : ENOSYS;
  return -1;
}
