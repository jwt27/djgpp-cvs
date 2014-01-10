/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2009 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/symlink.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <io.h>

int
mkstemp(char *_template)
{
  char tmp_name[FILENAME_MAX];
  char real_path[FILENAME_MAX];
  int  fd = -1;

  /* Make sure we create a non-exisiting file, even
     if race conditions exist with other processes.  */
  do {
    strcpy(tmp_name, _template);
    errno = 0;
  } while (mktemp(tmp_name) != NULL
           && __solve_symlinks(tmp_name, real_path)
           && (fd = _creatnew(real_path, 0, SH_DENYRW)) == -1
           && errno == EEXIST);

  if (fd == -1)
    errno = ENOENT;
  else
    strcpy(_template, tmp_name);

  return fd;
}
