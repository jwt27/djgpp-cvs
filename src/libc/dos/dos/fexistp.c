/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <errno.h>
#include <io.h>

int __file_exists(const char *fn)
{
  int olderr,attr;
  olderr = errno;
  attr = _chmod(fn, 0);
  errno = olderr;
  if(attr == -1)
    return 0;
  return 1;
}
