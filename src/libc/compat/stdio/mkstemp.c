/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <io.h>

int
mkstemp (char *_template)
{
  char tmp_name[FILENAME_MAX];
  int  fd = -1;

  /* Make sure we create a non-exisiting file, even
     if race conditions exist with other processes.  */
  do {
    strcpy(tmp_name, _template);
    errno = 0;
  } while (mktemp (tmp_name) != NULL
	   && (fd = _creatnew(tmp_name, 0, SH_DENYRW)) == -1
	   && errno == EEXIST);

  if (fd == -1)
    errno = ENOENT;
  else
    strcpy(_template, tmp_name);

  return fd;
}
