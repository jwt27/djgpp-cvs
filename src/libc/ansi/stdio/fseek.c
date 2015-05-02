/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <unistd.h>
#include <libc/file.h>
#include <fcntl.h>
#include <libc/dosio.h>
#include <libc/fd_props.h>

int
fseek(FILE *f, long offset, int ptrname)
{
  const int fd = fileno(f);
  long int p = -1;			/* can't happen? */

  /* If this is a FILE for a directory, we have no concept of position.
   * The stream I/O functions cannot be used to read/write a FILE
   * for directories. So, just return position 0. */
  if (__get_fd_flags(fd) & FILE_DESC_DIRECTORY)
    return 0;

  /* See comment in filbuf.c */
  f->_fillsize = 512;

  f->_flag &= ~_IOEOF;
  if (f->_flag & _IOREAD)
  {
    if (f->_base && !(f->_flag & _IONBF))
    {
      p = ftell(f);
      if (ptrname == SEEK_CUR)
      {
	offset += p;
	ptrname = SEEK_SET;
      }
      /* check if the target position is in the buffer and
        optimize seek by moving inside the buffer */
      if (ptrname == SEEK_SET && (f->_flag & (_IOUNGETC | _IORW)) == 0
          && p - offset <= f->_ptr - f->_base && offset - p <= f->_cnt)
      {
        f->_ptr += offset - p;
        f->_cnt += p - offset;
        return 0;
      }
    }

    if (f->_flag & _IORW)
      f->_flag &= ~_IOREAD;

    p = lseek(fd, offset, ptrname);
    f->_cnt = 0;
    f->_ptr = f->_base;
    f->_flag &= ~_IOUNGETC;
  }
  else if (f->_flag & (_IOWRT | _IORW))
  {
    p = fflush(f);
    return lseek(fd, offset, ptrname) == -1 || p == EOF ? -1 : 0;
  }
  return p == -1 ? -1 : 0;
}
