/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <libc/file.h>
#include <libc/local.h>
#include <libc/dosio.h>
#include <libc/fd_props.h>

FILE *
fopen(const char *file, const char *mode)
{
  FILE *f;
  int fd, rw, oflags = 0;
  char tbchar;

  if (file == 0)
    return 0;
  if (mode == 0)
    return 0;

  f = __alloc_file();
  if (f == NULL)
    return NULL;

  rw = (mode[1] == '+') || (mode[1] && (mode[2] == '+'));

  switch (*mode)
  {
  case 'a':
    oflags = O_CREAT | (rw ? O_RDWR : O_WRONLY) | O_APPEND;
    break;
  case 'r':
    oflags = rw ? O_RDWR : O_RDONLY;
    break;
  case 'w':
    oflags = O_TRUNC | O_CREAT | (rw ? O_RDWR : O_WRONLY);
    break;
  default:
    return (NULL);
  }
  if (mode[1] == '+')
    tbchar = mode[2];
  else
    tbchar = mode[1];
  if (tbchar == 't')
    oflags |= O_TEXT;
  else if (tbchar == 'b')
    oflags |= O_BINARY;
  else
    oflags |= (_fmode & (O_TEXT|O_BINARY));

  fd = open(file, oflags, 0666);
  if (fd < 0)
    return NULL;

  f->_cnt = 0;
  f->_file = fd;
  f->_bufsiz = 0;
  if (rw)
    f->_flag = _IORW;
  else if (*mode == 'r')
    f->_flag = _IOREAD;
  else
    f->_flag = _IOWRT;

  if (*mode == 'a')
  {
    llseek(fd, 0LL, SEEK_END);
  }

  f->_base = f->_ptr = NULL;

  /* If this is a FILE for a directory, we need to make sure certain
   * flags are clear and certain flags are set. Namely:
   *
   * - The read flag should be clear, since reads aren't allowed.
   * - The write flag should be clear, since writes aren't allowed.
   * - The read-write flag should be clear, because of the above.
   * - The EOF flag should be set, so that certain functions
   *   fail reads and writes. (Easier than modifying the functions).
   */
  if (__get_fd_flags(fd) & FILE_DESC_DIRECTORY)
  {
    f->_flag &= ~(_IORW|_IOREAD|_IOWRT);
    f->_flag |= _IOEOF;
  }

  return f;
}
