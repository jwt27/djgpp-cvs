/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <dpmi.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/fsext.h>

static int
is_used_fd(int fd)
{
  __dpmi_regs regs;

  regs.x.ax = 0x4400;
  regs.x.bx = fd;
  __dpmi_int(0x21, &regs);
  if (regs.x.flags & 1)
    return 0;

  return 1;
}

int
fcntl(int fd, int cmd, ...)
{
  int tofd, open_max;
  va_list ap;
  __FSEXT_Function *func = __FSEXT_get_function(fd);
  if (func)
  {
    int rv;
    if (func(__FSEXT_fcntl, &rv, &fd))
      return rv;
  }

  switch (cmd)
  {
  case F_DUPFD:
    va_start(ap, cmd);
    tofd = va_arg(ap, int);
    va_end(ap);

    open_max = getdtablesize();
    if (tofd < 0 || tofd >= open_max)
    {
      errno = EINVAL;
      return -1;
    }
    while (tofd < open_max)
    {
      if (! is_used_fd(tofd))
	break;
      tofd++;
    }

    if (tofd >= open_max)
    {
      errno = EMFILE;
      return -1;
    }

    return dup2(fd, tofd);
    
  case F_GETFD:
    /* DOS only passes the first 20 handles to child programs.  In
       addition, handles 19 and 18 will be closed by the stub of the
       child program (if it is a DJGPP program).

       FIXME: we should look at the no-inherit bit stashed in the SFT
       entry pointed to by the handle, since some of the first 18
       handles could have been opened with a no-inherit bit.  */
    return fd >= 18 ? FD_CLOEXEC : 0;
  case F_SETFD:
    if ((fd < 18) ^ ((cmd & FD_CLOEXEC) != 0))
      return 0;
    else
      {
	errno = ENOSYS;
	return -1;
      }
  case F_GETFL:
    return 0;	/* FIXME: should use the data in the SFT */
  case F_SETFL:
    errno = ENOSYS;
    return -1;
  case F_GETLK:
  case F_SETLK:
  case F_SETLKW:
    errno = ENOSYS;
    return -1;
  }
  errno = ENOSYS;
  return -1;
}
