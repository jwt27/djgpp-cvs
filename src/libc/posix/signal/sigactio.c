/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <signal.h>
#include <errno.h>

int
sigaction(int _sig, const struct sigaction *_act, struct sigaction *_oact)
{
  /* note that sigaction always fails for SIGKILL */
  if (_oact)
  {
    _oact->sa_flags = 0;
    _oact->sa_handler = signal(_sig, SIG_IGN);
    if (sigemptyset(&_oact->sa_mask) != 0 || _oact->sa_handler == SIG_ERR)
    {
      errno = EINVAL;
      return -1;
    }
    signal(_sig, _oact->sa_handler);
  }
  if (_act)
  {
    if (signal(_sig, _act->sa_handler) == SIG_ERR)
    {
      errno = EINVAL;
      return -1;
    }
  }
  return 0;
}
