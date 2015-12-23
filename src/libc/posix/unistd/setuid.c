/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <errno.h>

int
setuid(uid_t _uid)
{
  if (_uid != getuid())
  {
    errno = EPERM;
    return -1;
  }
  return 0;
}
