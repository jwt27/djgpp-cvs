/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdarg.h>
#include <sys/stat.h>		/* For stat() */
#include <fcntl.h>		/* For O_RDONLY, etc. */
#include <unistd.h>		/* For read(), write(), etc. */
#include <limits.h>		/* For PATH_MAX */
#include <utime.h>		/* For utime() */
#include <errno.h>		/* For errno */
#include <sys/fsext.h>
#include <libc/fsexthlp.h>

/* Of course, DOS can't really do a link.  We just do a copy instead,
   which is as close as DOS gets.  Alternatively, we could always fail
   and return -1.  I think this is slightly better. */
int
link(const char *path1, const char *path2)
{
  struct stat statbuf1, statbuf2;
  struct utimbuf times;
  char buf[16384];
  int fd1, fd2, nbyte, status1, status2,rv;

  /* Fail if either path is null */
  if (path1 == NULL || path2 == NULL)
  {
    errno = EFAULT;
    return -1;
  }
  if (*path1 == '\0' || *path2 == '\0')
  {
    errno = ENOENT;
    return -1;
  }

  /* see if a file system extension implements the link */
  if (__FSEXT_call_open_handlers_wrapper(__FSEXT_link, &rv, path1, path2))
    return rv;

  /* Fail if path1 does not exist - stat() will set errno */
  if (stat(path1, &statbuf1) < 0) return -1;

  /* Fail if path1 is not a regular file */
  if (!S_ISREG(statbuf1.st_mode))
  {
    errno = EPERM;
    return -1;
  }

  /* Fail if unable to open path1 - open() will set errno */
  fd1 = open(path1, O_RDONLY | O_BINARY);
  if (fd1 < 0) return -1;

  /* Fail if unable to create path2 - open() will set errno */
  fd2 = open(path2, O_WRONLY | O_BINARY | O_CREAT | O_EXCL, 0600);
  if (fd2 < 0)
  {
    (void) close(fd1);
    return -1;
  }

  /* Fail if path1 and path2 are on different devices */
  if (fstat(fd2, &statbuf2) < 0) return -1;
  if (statbuf1.st_dev != statbuf2.st_dev)
  {
    (void)close(fd1);
    (void)close(fd2);
    (void)unlink(path2);
    errno = EXDEV;
    return -1;
  }

  /* Copy path1 to path2 */
  do
  {
    nbyte = read(fd1, buf, sizeof buf);
    if (nbyte <= 0) break;
    if (write(fd2, buf, nbyte) != nbyte) nbyte = -1;
  }
  while (nbyte > 0);

  /* Fail if the copy failed or we can't clean up */
  status1 = close(fd1);
  status2 = close(fd2);
  if (nbyte < 0 || status1 < 0 || status2 < 0)
  {
    (void) unlink(path2);
    return -1;
  }

  /* Success! */

  /* Set the mode to match the original, ignoring errors */
  (void) chmod(path2, statbuf1.st_mode);

  /* Set the file time to match the original, ignoring errors */
  times.actime = statbuf1.st_atime;
  times.modtime = statbuf1.st_mtime;
  (void) utime(path2, &times);

  return 0;
}
