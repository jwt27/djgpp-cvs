/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <io.h>

int
ftruncate(int fd, off_t where)
{
  off_t here = lseek(fd, 0, SEEK_CUR);
  int retval = 0;

  if (here == -1)
    return -1;
  if (lseek(fd, where, SEEK_SET) == -1)
    return -1;
  if (_write(fd, 0, 0) < 0)
    retval = -1;
  lseek(fd, here, SEEK_SET);
  return retval;
}
