/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include "dirstruc.h"

struct dirent *
readdir(DIR *dir)
{
  int done;
  int oerrno = errno;
  int mbsize;

  if (dir->need_fake_dot_dotdot)
  {
    /* Fake out . and .. on /; see opendir for comments */
    dir->need_fake_dot_dotdot --;
    if (dir->need_fake_dot_dotdot)
      strcpy(dir->de.d_name, ".");
    else
      strcpy(dir->de.d_name, "..");
    dir->de.d_namlen = strlen(dir->de.d_name);
    return &(dir->de);
  }

  if (dir->num_read)
    done = findnext(&dir->ff);
  else
  {
    int ff_flags = FA_ARCH|FA_RDONLY|FA_DIREC|FA_SYSTEM;
    if (!(dir->flags & __OPENDIR_NO_HIDDEN))
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
  if (!(dir->flags & __OPENDIR_PRESERVE_CASE))
  {
    char *cp, fsh[13];

    if (!strcmp(_lfn_gen_short_fname(dir->ff.ff_name, fsh), dir->ff.ff_name))
      for (cp=dir->ff.ff_name; *cp; cp++)
#if 1
	{
	  mbsize = mblen (cp, MB_CUR_MAX);
	  if (mbsize > 1)
	    {
	      cp += mbsize - 1;
	      continue;
	    }
	  else if (*cp >= 'A' && *cp <= 'Z')
	    *cp += 'a' - 'A';
	}
#else
	if (*cp >= 'A' && *cp <= 'Z')
	  *cp += 'a' - 'A';
#endif
  }
  strcpy(dir->de.d_name, dir->ff.ff_name);
  dir->de.d_namlen = strlen(dir->de.d_name);
  return &dir->de;
}

