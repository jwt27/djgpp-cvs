/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/symlink.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <io.h>
#include <dir.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>

int access(const char *fn, int flags)
{
  int attr;
  char file_name[FILENAME_MAX];

  if (!__solve_symlinks(fn, file_name))
     return -1;

  attr = _chmod(file_name, 0);

  if (attr == -1) {
    struct ffblk ff;
    char fixed_path[FILENAME_MAX];

    /* Root directories on some non-local drives (e.g. CD-ROM)
       might fail `_chmod'.  `findfirst' to the rescue.  */
    _fixpath(file_name, fixed_path);
    if (fixed_path[1] == ':' && fixed_path[2] == '/' && fixed_path[3] == 0)
      {
        char *fp = fixed_path + 3;

        *fp++ = '*';
        *fp++ = '.';
        *fp++ = '*';
        *fp++ = '\0';
        /* Use _lfn_close_handle so we don't lose a handle.  */
        if (findfirst(fixed_path, &ff, FA_DIREC) == 0)
        {
          if (strcmp(ff.lfn_magic, "LFN32") == 0)
            _lfn_find_close(ff.lfn_handle);
          return 0;
        }
      }

    /* Devices also fail `_chmod'; some programs won't write to
       a device unless `access' tells them they are writeable.  */
    if (findfirst(file_name, &ff, FA_RDONLY | FA_ARCH) == 0
	&& (ff.ff_attrib & 0x40) == 0x40
	&& (flags & (X_OK | D_OK)) == 0)
    {
      if (strcmp(ff.lfn_magic, "LFN32") == 0)
        _lfn_find_close(ff.lfn_handle);
      return 0;
    }

    errno = ENOENT;
    return -1;
  }
 
  if (attr & 0x10)		/* directory? */
      return 0;			/* directories always OK */
  if (flags & D_OK)
  {
    errno = EACCES;
    return -1;			/* not a directory */
  }

  if ((flags & W_OK) && (attr & 3)) /* read-only, system or hidden */
  {
    errno = EACCES;
    return -1;			/* not writable */
  }

  if (flags & X_OK)
  {
    if (!_is_executable(file_name, 0, 0))
    {
      errno = EACCES;
      return -1;
    }
  }

  return 0;			/* everything else is OK */
}
