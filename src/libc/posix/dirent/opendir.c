/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
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
  if (!(__opendir_flags & __OPENDIR_PRESERVE_CASE) && _preserve_fncase())
    dir->flags |= __OPENDIR_PRESERVE_CASE;

  /* Make absolute path */
  _fixpath(name, dir->name);

  /* If we're doing opendir of the root directory, we need to
     fake out the . and .. entries, as some unix programs (like
     mkisofs) expect them and fail if they don't exist */
  dir->need_fake_dot_dotdot = 0;
  if (dir->name[1] == ':' && dir->name[2] == '/' && dir->name[3] == 0)
  {
    /* see if findfirst finds "." anyway */
    int done = findfirst(dir->name, &dir->ff, FA_ARCH|FA_RDONLY|FA_DIREC|FA_SYSTEM);
    if (done || strcmp(dir->ff.ff_name, "."))
      dir->need_fake_dot_dotdot = 2;
  }

  /* Ensure that directory to be accessed exists */
  if (access(dir->name, D_OK))
  {
    free(dir->name);
    free(dir);
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
