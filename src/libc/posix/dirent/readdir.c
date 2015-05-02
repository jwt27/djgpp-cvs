/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2010 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2008 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <io.h>
#include <libc/symlink.h>
#include "dirstruc.h"

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
    if ((__opendir_flags & __OPENDIR_NO_D_TYPE) == 0)
      dir->de.d_type = DT_DIR;
    else
      dir->de.d_type = DT_UNKNOWN;
    return &(dir->de);
  }

  if (dir->num_read)
    done = findnext(&dir->ff);
  else
  {
    char dir_name[FILENAME_MAX + 1];
    int ff_flags = FA_ARCH|FA_RDONLY|FA_DIREC|FA_SYSTEM;
    if (!(dir->flags & __OPENDIR_NO_HIDDEN))
      ff_flags |= FA_HIDDEN;
    if (dir->flags & __OPENDIR_FIND_LABEL)
      ff_flags |= FA_LABEL;
    APPEND_STAR_DOT_STAR(dir_name, dir->name);
    done = findfirst(dir_name, &dir->ff, ff_flags);
  }
  if (done)
  {
    if (errno == ENMFILE)
      errno = oerrno;
    return 0;
  }
  dir->num_read++;
  if (!(dir->flags & __OPENDIR_PRESERVE_CASE))
  {
    char *cp;

    if (_is_DOS83(dir->ff.ff_name))
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
  if ((__opendir_flags & __OPENDIR_NO_D_TYPE) == 0)
    {
      unsigned char attrib = dir->ff.ff_attrib;

      if ((attrib & FA_DIREC) == FA_DIREC)
	dir->de.d_type = DT_DIR;
      else if ((attrib & FA_LABEL) == FA_LABEL)
	dir->de.d_type = DT_LABEL;
      else if ((attrib & 0x40) == 0x40)
	dir->de.d_type = DT_CHR;
      else if (dir->ff.ff_fsize == _SYMLINK_FILE_LEN)
      {
	int fhandle = -1;
	char fname[FILENAME_MAX];

	dir->de.d_type = DT_REG;
	strcat(strcat(strcpy(fname, dir->name), "/"), dir->de.d_name);
	/* Opening a file is expensive, but hopefully there won't be
	   so many files whose length is 510 bytes.  */
	fhandle = _open(fname, O_RDONLY);
	if (fhandle < 0)
	  fhandle = _open(fname, O_RDONLY | SH_DENYNO);
	if (fhandle >= 0)
	{
	  char sbuf[_SYMLINK_FILE_LEN];

	  if (__internal_readlink(NULL, fhandle, sbuf, _SYMLINK_FILE_LEN) > 0)
	    dir->de.d_type = DT_LNK;
	  _close(fhandle);
          errno = oerrno;
	}
      }
      /* FIXME: anything else DOS can return? */
      else
	dir->de.d_type = DT_REG;
    }
  else
    dir->de.d_type = DT_UNKNOWN;
  return &dir->de;
}

