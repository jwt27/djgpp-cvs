/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

/* ARGSUSED */
int
mknod(const char *path, mode_t mode, dev_t dev)
{
  if (S_ISREG(mode))
  {
    int e  = errno;
    int fd = open(path, O_CREAT | O_EXCL, S_IWUSR);
 
    if (fd == -1)
      return fd;
    close(fd);
    errno = e;
    return 0;
  }
  else if (S_ISCHR(mode))
  {
    struct stat statbuf;
 
    if (stat(path, &statbuf) == 0 &&
        S_ISCHR(statbuf.st_mode)  &&
        statbuf.st_dev == dev)
    {
      errno = EEXIST;
      return -1;
    }
  }
 
  errno = EACCES;
  return -1;
}
