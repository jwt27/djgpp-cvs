/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <unistd.h>
#include <libc/file.h>
#include <libc/fd_props.h>

void rewind(FILE *f)
{
  const int fd = fileno(f);

  /* If this is a FILE for a directory, we must maintain its EOF flag.
   * Just return. */
  if (__get_fd_flags(fd) & FILE_DESC_DIRECTORY)
    return;

  fflush(f);
  lseek(fd, 0L, SEEK_SET);
  f->_fillsize = 512;	/* See comment in filbuf.c */
  f->_cnt = 0;
  f->_ptr = f->_base;
  f->_flag &= ~(_IOERR | _IOEOF);
  if (f->_flag & _IORW)
    f->_flag &= ~(_IOREAD | _IOWRT);
}
