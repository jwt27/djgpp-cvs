/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <signal.h>
#include <errno.h>

extern sigset_t __sigprocmask_pending_signals;

int
sigpending(sigset_t *set)
{
  if (set == (sigset_t *)0)
    {
      errno = EFAULT;
      return -1;
    }

  *set = __sigprocmask_pending_signals;
  return 0;
}
