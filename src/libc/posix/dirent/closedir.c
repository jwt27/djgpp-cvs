/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include "dirstruc.h"

int
closedir(DIR *dir)
{
  int retval = 0;
  int e = errno;

  errno = 0;
  rewinddir(dir);	/* under LFN this closes the search handle */
  if (errno == 0)
    errno = e;
  else
    retval = -1;
  free(dir->name);
  free(dir);
  return retval;
}
