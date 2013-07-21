/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <go32.h>
#include <libc/file.h>
#include <io.h>
#include <libc/fd_props.h>

int
_flsbuf(int c, FILE *f)
{
  char *base;
  ssize_t n;
  size_t rn;
  char c1;
  size_t size;

  if (f->_flag & _IORW)
  {
    f->_flag |= _IOWRT;
    f->_flag &= ~(_IOEOF | _IOREAD);
  }

  if (!(f->_flag & _IOWRT))
  {
    f->_flag |= _IOERR;
    return EOF;
  }

  /* No-op for full string buffers */
  if (f->_flag & _IOSTRG)
    return c;

  /* if the buffer is not yet allocated, allocate it */
  if ((base = f->_base) == NULL && !(f->_flag & _IONBF))
  {
    size = __tb_size;
    if ((f->_base = base = malloc (size)) == NULL)
    {
      f->_flag |= _IONBF;
      f->_flag &= ~(_IOFBF | _IOLBF);
    }
    else
    {
      f->_flag |= _IOMYBUF;
      f->_cnt = f->_bufsiz = size;
      f->_ptr = base;
      rn = 0;
      if (f == stdout && isatty(fileno(stdout)))
        f->_flag |= _IOLBF;
    }
  }

  if (f->_flag & _IOLBF)
  {
    /* in line-buffering mode we get here on each character */
    *f->_ptr++ = c;
    rn = f->_ptr - base;
    if (c == '\n' || rn >= f->_bufsiz)
    {
      /* time for real flush */
      f->_ptr = base;
      f->_cnt = 0;
    }
    else
    {
      /* we got here because _cnt is wrong, so fix it */
      /* Negative _cnt causes all output functions
         to call _flsbuf for each character, thus realizing line-buffering */
      f->_cnt = -rn;
      return c;
    }
  }
  else if (f->_flag & _IONBF)
  {
    c1 = c;
    rn = 1;
    base = &c1;
    f->_cnt = 0;
  }
  else /* _IOFBF */
  {
    rn = f->_ptr - base;
    f->_ptr = base;
    f->_cnt = f->_bufsiz;
  }
  while (rn > 0)
  {
    /* If termios hooked this handle, call the termios hook.
       We only do this with handles marked by putc and fwrite,
       to prevent double conversion of NL to CR-LF and avoid
       messing up the special termios conversions the user
       might have requested for CR and NL.  */
    if ((f->_flag & _IOTERM) == 0
        || __libc_write_termios_hook == NULL
        || __libc_write_termios_hook(fileno(f), base, rn, &n) == 0)
    {
      int fd = fileno(f);
      if (__get_fd_flags(fd) & FILE_DESC_APPEND)
      {
        int save_errno = errno; /* We don't want llseek()'s setting 
                                   errno to remain. */
        if (llseek(fd, 0, SEEK_END) == -1)
        {
          errno = save_errno;
          return EOF;
        }
      }
      n = _write(fd, base, rn);
    }
    if (n <= 0)
    {
      f->_flag |= _IOERR;
      return EOF;
    }
    rn -= n;
    base += n;
  }

  if (!(f->_flag & (_IOLBF | _IONBF)))
  {
    f->_cnt--;
    *f->_ptr++ = c;
  }

  return c;
}
