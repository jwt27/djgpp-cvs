/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2008 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/symlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libc/dosio.h>
#include <dpmi.h>
#include "dirstruc.h"

#define IS_ROOT_DIR(path)  (((path)[1] == ':') && ((path)[2] == '/') && ((path)[3] == '\0'))

#define APPEND_STAR_DOT_STAR(dst, src)  \
  do {                                  \
    int _i;                             \
                                        \
    for (_i = 0; (src)[_i]; _i++)       \
      (dst)[_i] = (src)[_i];            \
    (dst)[_i++] = '/';                  \
    (dst)[_i++] = '*';                  \
    (dst)[_i++] = '.';                  \
    (dst)[_i++] = '*';                  \
    (dst)[_i++] = '\0';                 \
  } while(0)


void
_lfn_find_close(int handle)
{
  __dpmi_regs r;

  r.x.flags = 1;  /* Always set CF before calling a 0x71NN function. */
  r.x.bx = handle;
  r.x.ax = 0x71a1;
  __dpmi_int(0x21, &r);
  if ((r.x.flags & 1) || (r.x.ax == 0x7100))
  {
    /*  Never assume that the complete LFN API is implemented,
        so check that AX != 0x7100.  E.G.: MSDOS 6.22 and DOSLFN 0.40.
        If not supported fail.  */
    errno = __doserr_to_errno(r.x.ax);
  }
}

void
__set_need_fake_dot_dotdot(DIR *dir)
{
  int oerrno = errno;

  dir->need_fake_dot_dotdot = 0;
  if (IS_ROOT_DIR(dir->name))
  {    
    /* see if findfirst finds "." anyway */
    char dir_name[FILENAME_MAX + 1];

    APPEND_STAR_DOT_STAR(dir_name, dir->name);
    if (findfirst(dir_name, &dir->ff, FA_ARCH|FA_RDONLY|FA_DIREC)
	|| strcmp(dir->ff.ff_name, "."))
    {
      dir->need_fake_dot_dotdot = 2;

      /* Restore errno in certain cases. findfirst() will fail for empty
	 root directories on drives, but this should not be considered
	 an error. */
      if ((errno == ENOENT) || (errno == ENMFILE))
	errno = oerrno;
    }

    if (_USE_LFN && dir->ff.lfn_handle)
    {
      _lfn_find_close(dir->ff.lfn_handle);
      dir->ff.lfn_handle = 0;
    }
  }
}

DIR *
opendir(const char *name)
{
  int length;
  DIR *dir;
  char name_copy[FILENAME_MAX + 1];

  if (!__solve_symlinks(name, name_copy))
     return NULL;
  
  dir = (DIR *)malloc(sizeof(DIR));
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
  _fixpath(name_copy, dir->name);

  /* Ensure that directory to be accessed exists */
  if (access(dir->name, D_OK))
  {
    free(dir->name);
    free(dir);
    return 0;
  }

  /* Strip trailing slashes.
     The "/ *.*" is no longer part of the directory
     name but is appended in every libc function
     that requires it.  */
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
  dir->name[length++] = 0;

  /* If we're doing opendir of the root directory, we need to
     fake out the . and .. entries, as some unix programs (like
     mkisofs) expect them and fail if they don't exist */
  __set_need_fake_dot_dotdot(dir);

  return dir;
}
