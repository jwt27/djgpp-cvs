/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <io.h>
#include <libc/file.h>
#include <libc/local.h>
#include <libc/symlink.h>

FILE *
tmpfile(void)
{
  int  temp_fd;
  FILE *f;
  char *temp_name = tmpnam(0);
  char *n_t_r = (char *)malloc(L_tmpnam);
  char  real_path[FILENAME_MAX];

  if (!n_t_r)
    return 0;

  /* We could have a race condition, whereby another program
     (in another virtual machine, or if the temporary file is
     in a directory which is shared via a network) opens the
     file returned by `tmpnam' between the call above and the
     moment when we actually open the file below.  This loop
     retries the call to `tmpnam' until we actually succeed
     to create the file which didn't exist before.  */

  do {
    /* Call to tmpnam() might have symlinks in returned path, however
     * _creatnew does not support them.  So resolve them here.
     */
    if (!__solve_symlinks(temp_name, real_path))
       return 0;
    errno = 0;
    temp_fd = _creatnew(real_path, 0, SH_DENYRW);
  } while (temp_fd == -1
	   && errno != ENOENT && errno != EMFILE
	   && (temp_name = tmpnam(0)) != 0);

  if (temp_name == 0 || temp_fd == -1)
  {
    free(n_t_r);
    return 0;
  }

  f = fdopen(temp_fd, "wb+");
  if (f)
  {
    f->_flag |= _IORMONCL;
    f->_name_to_remove = n_t_r;
    strcpy(f->_name_to_remove, temp_name);
  }
  else
  {
    close(temp_fd);
    remove(real_path);
    free(n_t_r);
  }
  return f;
}
