/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <dirent.h>
#include <fcntl.h>
#include "dirstruc.h"

void
rewinddir(DIR *dir)
{
  /* If we are using LFN-aware functions, close the handle used by
     Windows 9X during the search (readdir will open a new one).  */
  if (_USE_LFN && dir->ff.lfn_handle)
  {
    _lfn_find_close(dir->ff.lfn_handle);
    dir->ff.lfn_handle = 0;	/* 0 means it's closed */
  }

  /* Recompute need_fake_dot_dotdot member.  See comments in opendir.c.  */
  __set_need_fake_dot_dotdot(dir);
  dir->num_read = 0;
}
