/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <stddef.h>
#include <termios.h>

speed_t
cfgetispeed (const struct termios *termiosp)
{
  /* check arguments */
  if (termiosp == NULL)
    {
      errno = EINVAL;
      return (speed_t) -1;
    }

  return termiosp->c_ispeed;
}
