/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include "dirstruc.h"

DIR *
opendir(const char *name)
{
  int length;
  DIR *dir = (DIR *)malloc(sizeof(DIR));
  if (dir == 0)
    return 0;
  dir->num_read = 0;
  dir->name = (char *)malloc(PATH_MAX);
  if (dir->name == 0)
  {
    free(dir);
    return 0;
  }

  dir->flags = __opendir_flags;

  /* Make absolute path */
  _fixpath (name, dir->name);

  /* Ensure that directory to be accessed exists */
  if (access(dir->name, D_OK))
  {
    free (dir->name);
    free (dir);
    return 0;
  }

  /* Strip trailing slashes, so we can append "/ *.*" */
  length = strlen(dir->name);
  while (1)
  {
    if (length == 0) break;
    length--;
    if (dir->name[length] == '/' ||
	dir->name[length] == '\\')
      dir->name[length] = '\0';
    else
    {
      length++;
      break;
    }
  }

  dir->name[length++] = '/';
  dir->name[length++] = '*';
  dir->name[length++] = '.';
  dir->name[length++] = '*';
  dir->name[length++] = 0;
  return dir;
}
