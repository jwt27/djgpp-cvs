/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <fcntl.h>

int
creat(const char *path, mode_t mode)
{
  return open(path, O_WRONLY|O_CREAT|O_TRUNC, mode);
}
