/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <io.h>

int
ftruncate(int fd, off_t where)
{
  if (lseek(fd, where, 0) == -1)
    return -1;
  if (_write(fd, 0, 0) < 0)
    return -1;
  return 0;
}
