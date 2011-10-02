/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <io.h>
#include <sys/fsext.h>
#include <libc/fsexthlp.h>
#include <libc/fd_props.h>
#include <libc/getdinfo.h>
#include <fcntl.h>
#include <dpmi.h>
#include <go32.h>
#include <libc/farptrgs.h>

static int
get_current_mode (const int fd)
{
  __dpmi_regs r;
  int         mode = 0; /* Fail by default */

  if (_USE_LFN)
  {
    r.x.flags = 1;   /* Always set CF before calling a 0x71NN function. */
    r.x.ax = 0x71a6; /* File info by handle */
    r.x.bx = fd;
    r.x.ds = __tb >> 4;
    r.x.dx = 0;

    __dpmi_int(0x21, &r);

    if (!(r.x.flags & 1) && (r.x.ax != 0x7100))
    {
      /*  Never assume that the complete LFN API is implemented,
          so check that AX != 0x7100.  E.G.: MSDOS 6.22 and DOSLFN 0.40.
          If not supported fail.  */
      int attr = _farpeekl(_dos_ds, __tb);

      mode = S_IRUSR; /* Files are always readable. */
      if ((attr & 1) == 0)
	mode |= S_IWUSR;
    }
  }

  return(mode);
}

int
fchmod (int fd, mode_t mode)
{
  __FSEXT_Function    *func     = __FSEXT_get_function(fd);
  const char          *filename = __get_fd_name(fd);
  const unsigned long  flags    = __get_fd_flags(fd);
  int                  dev_info;
  int                  current_mode;
  int                  rv;

  if (   func
      && __FSEXT_func_wrapper(func, __FSEXT_fchmod, &rv, fd, mode))
      return(rv);

  /* Check that it's a valid file descriptor. */
  dev_info = _get_dev_info(fd);
  if (dev_info == -1)
    return(-1);

  /* Is this a pipe? Disallow changing the mode on pipes. */
  if (flags & FILE_DESC_PIPE) {
    errno = EINVAL;
    return(-1);
  }

  /* Is this a redirected standard handle: stdin, stdout, stderr?
   * I.e.: are the standard handles pipes? Disallow changing the mode
   * on pipes. */
  switch(fd) {
  case STDIN_FILENO:
  case STDOUT_FILENO:
  case STDERR_FILENO:
    if (isatty(fd) == 0) {
      errno = EINVAL;
      return(-1);
    }
    break;

  default:
    break;
  }

  /* Is this a character device? If so, silently ignore the request. */
  if (dev_info & _DEV_CDEV)
    return 0;

  /* Get the current mode. If it's the same as those requested,
   * just return. */
  /* NB: Only implemented toggle is write/nowrite */
  current_mode = get_current_mode(fd);
  if (current_mode && (current_mode == (mode & (S_IRUSR|S_IWUSR))))
    return 0;

  /* It's not a device and we don't have the filename. So we can only
   * fail. */
  if (filename == NULL) {
    errno = ENOSYS;
    return(-1);
  }

  return(chmod(filename, mode));
}
