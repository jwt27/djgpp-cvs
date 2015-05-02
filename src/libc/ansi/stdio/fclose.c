/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <libc/file.h>
#include <libc/fd_props.h>

int
fclose(FILE *f)
{
  const int fd = fileno(f);
  int r = EOF;

  if (!f)
    return r;

  /* A FILE for a directory won't have any of the read or write flags
   * set. But we still want to tidy it up. */
  if ((f->_flag & (_IOREAD | _IOWRT | _IORW) && !(f->_flag & _IOSTRG))
      || (__get_fd_flags(fd) & FILE_DESC_DIRECTORY))
  {
    r = fflush(f);
    if (close(fd) < 0)
      r = EOF;
    if (f->_flag & _IOMYBUF)
      free(f->_base);
  }

  if (f->_flag & _IORMONCL && f->_name_to_remove)
  {
    remove(f->_name_to_remove);
    free(f->_name_to_remove);
    f->_name_to_remove = 0;
  }

  f->_cnt = 0;
  f->_base = NULL;
  f->_ptr = NULL;
  f->_bufsiz = 0;
  f->_flag = 0;
  f->_file = -1;
  return r;
}
