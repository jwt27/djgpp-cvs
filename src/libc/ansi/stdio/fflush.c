/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <libc/file.h>
#include <io.h>
#include <libc/fd_props.h>

int
fflush(FILE *f)
{
  char *base;
  ssize_t n;
  size_t rn;

  if (f == NULL)
  {
    int e = errno;

    errno = 0;
    _fwalk((void (*)(FILE *))fflush);
    if (errno)
      return EOF;
    errno = e;
    return 0;
  }

  if (__get_fd_flags(fileno(f)) & FILE_DESC_APPEND)
  {
    int save_errno = errno; /* We don't want llseek()'s setting
                               errno to remain. */
    if (llseek(fileno(f), 0LL, SEEK_END) == -1)
    {
      errno = save_errno;
      return -1;
    }
  }

  f->_flag &= ~_IOUNGETC;
  if ((f->_flag & (_IONBF | _IOWRT)) == _IOWRT
      && (base = f->_base) != NULL
      && (n = f->_ptr - base) > 0)
  {
    rn = n;
    f->_ptr = base;
    f->_cnt = (f->_flag & (_IOLBF | _IONBF)) ? 0 : f->_bufsiz;
    do {
      /* If termios hooked this handle, call the termios hook.
         We only do this with handles marked by putc and fwrite,
         to prevent double conversion of NL to CR-LF and avoid
         messing up the special termios conversions the user
         might have requested for CR and NL.  */
      if ((f->_flag & _IOTERM) == 0
          || __libc_write_termios_hook == NULL
          || __libc_write_termios_hook(fileno(f), base, rn, &n) == 0)
        n = _write(fileno(f), base, rn);
      if (n < 1)
      {
        f->_flag |= _IOERR;
        return EOF;
      }
      rn -= n;
      base += n;
    } while (rn > 0);
  }
  if (f->_flag & _IORW)
  {
    f->_cnt = 0;
    f->_flag &= ~(_IOWRT | _IOREAD);
    f->_ptr = f->_base;
  }
  return 0;
}
