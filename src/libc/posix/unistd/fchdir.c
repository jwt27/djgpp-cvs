/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <io.h>
#include <libc/fd_props.h>

int
fchdir (int fd)
{
  const char *filename = __get_fd_name(fd);
  const int   flags    = __get_fd_flags(fd);

  /* Check that it's a valid file descriptor. */
  if (_get_dev_info(fd) == -1)
    return(-1);

  /* Is it actually a directory? */
  if (((flags & FILE_DESC_DIRECTORY) == 0) || (filename == NULL)) {
    errno = ENOTDIR;
    return(-1);
  }

  return(chdir(filename));
}
