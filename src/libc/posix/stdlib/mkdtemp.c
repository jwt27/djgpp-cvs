/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
#include <libc/symlink.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

char *
mkdtemp(char *_template)
{
  char tmp_name[FILENAME_MAX];
  char real_path[FILENAME_MAX];
  int  rv = -1;

  do {
    strcpy(tmp_name, _template);
    errno = 0;
  } while (mktemp(tmp_name) != NULL
           && __solve_symlinks(tmp_name, real_path)
           && (rv = mkdir(real_path, S_IWUSR))
           && errno == EEXIST);

  if (rv == 0)
  {
    strcpy(_template, tmp_name);
    return _template;
  }
  else
    return NULL;
}
