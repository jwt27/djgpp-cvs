/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <errno.h>
 
/* MS-DOS couldn't care less about file ownerships, so we could
   always succeed.  At least fail for non-existent files
   and for devices.  */
 
int
chown(const char *path, uid_t owner __attribute__((__unused__)), 
                        gid_t group __attribute__((__unused__)))
{
  if (access(path, F_OK))       /* non-existent file */
  {
    errno = ENOENT;
    return -1;
  }
  return 0;
}
