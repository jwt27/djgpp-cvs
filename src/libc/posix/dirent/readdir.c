/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include "dirstruc.h"

struct dirent *
readdir(DIR *dir)
{
  int done;
  int oerrno = errno;
  if (dir->num_read)
    done = findnext(&dir->ff);
  else
  {
    int ff_flags = FA_ARCH|FA_RDONLY|FA_DIREC|FA_SYSTEM;
    if (dir->flags & __OPENDIR_FIND_HIDDEN)
      ff_flags |= FA_HIDDEN;
    if (dir->flags & __OPENDIR_FIND_LABEL)
      ff_flags |= FA_LABEL;
    done = findfirst(dir->name, &dir->ff, ff_flags);
  }
  if (done)
  {
    if (errno == ENMFILE)
      errno = oerrno;
    return 0;
  }
  dir->num_read ++;
  strcpy(dir->de.d_name, dir->ff.ff_name);
  dir->de.d_namlen = strlen(dir->de.d_name);
  if (!(dir->flags & __OPENDIR_PRESERVE_CASE))
  {
    char *cp;
    for (cp=dir->de.d_name; *cp; cp++)
      *cp = tolower(*cp);
  }
  return &dir->de;
}
