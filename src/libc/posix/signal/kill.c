/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <signal.h>
#include <errno.h>
#include <unistd.h>

int
kill(pid_t pid, int sig)
{
  if (pid == getpid()
      || pid == 0
      || pid == -1
      || pid == -getpid())
  {
    if (sig)
      raise(sig);
    return 0;
  }
  else
  {
    errno = ENOSYS;
    return -1;
  }
}
