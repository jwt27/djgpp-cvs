/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <signal.h>

int
sigaction(int _sig, const struct sigaction *_act, struct sigaction *_oact)
{
  if (_oact)
  {
    /* FIXME */
    _oact->sa_handler = SIG_DFL;
    sigemptyset(&_oact->sa_mask);
    _oact->sa_flags = 0;
  }
  if (_act)
  {
    signal(_sig, _act->sa_handler);
  }
  return 0;
}
