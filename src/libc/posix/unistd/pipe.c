/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <libc/fd_props.h>

/* Emulate a pipe using a temporary file.  */
int
pipe(int fildes[2])
{
  int ifd, ofd;
  char temp_name[FILENAME_MAX + 1];
  char *tname;

  tname = tmpnam(temp_name);
  if (tname == NULL)
    return -1;
    
  ofd = open(tname, O_WRONLY | O_CREAT | O_TRUNC | O_TEMPORARY,
             S_IWUSR);
  if (ofd < 0)
    return -1;
    
  ifd = open(tname, O_RDONLY | O_TEMPORARY);
  if (ifd < 0)
  {
    close(ofd);
    return -1;
  }

  if (__has_fd_properties(ofd))
    __set_fd_flags(ofd, FILE_DESC_PIPE);
  if (__has_fd_properties(ifd))
    __set_fd_flags(ifd, FILE_DESC_PIPE);

  fildes[0] = ifd;
  fildes[1] = ofd;
    
  return 0;
}
