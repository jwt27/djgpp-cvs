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
  case F_SETFD:
  case F_GETFL:
  case F_SETFL:
    return 0;
  case F_GETLK:
  case F_SETLK:
  case F_SETLKW:
    return -1;
  }
  return -1;
}
