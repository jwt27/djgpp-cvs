/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/symlink.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/fsext.h>
#include <libc/fsexthlp.h>
 
/* MS-DOS couldn't care less about file ownerships, so we could
   always succeed.  At least fail for non-existent files
   and for devices.  Let FSEXTs handle it, if they want. */
 
int
chown(const char *path, uid_t owner, gid_t group)
{
  char real_name[FILENAME_MAX];
  int rv;

  if (access(path, F_OK))       /* non-existent file */
  {
    errno = ENOENT;
    return -1;
  }

  if (!__solve_symlinks(path, real_name))
     return -1;

  /* See if a file system extension has a hook for this file. */
  if (__FSEXT_call_open_handlers_wrapper(__FSEXT_chown, &rv,
					 real_name, owner, group))
    return rv;

  return 0;
}
