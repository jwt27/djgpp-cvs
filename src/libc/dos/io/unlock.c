/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <io.h>
#include <errno.h>

int
unlock(int fd, long offset, long length)
{
  int ret = _dos_unlock(fd, offset, length);
  /* change return code because Borland does it this way */
  if (ret == 0x21)
    ret = EACCES;
  if (ret != 0)
  {
    errno = ret;
    return -1;
  }
  else
    return 0;
}

