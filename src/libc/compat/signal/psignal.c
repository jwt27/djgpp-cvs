/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */

#include <signal.h>
#include <stdio.h>

void
psignal (int sig, const char *msg)
{
  if (sig >= 0 && sig < NSIG)
    fprintf (stderr, "%s: %s\n", msg, sys_siglist[sig]);
  else
    fprintf (stderr, "%s: signal %d\n", msg, sig);
}
