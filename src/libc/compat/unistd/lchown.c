/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <errno.h>
 
/* MS-DOS couldn't care less about file ownerships, so we could
   always succeed.  At least fail for non-existent files
   and for devices.  */
 
int
lchown(const char *path, uid_t owner, gid_t group)
{
  if (!__file_exists(path))       /* non-existent file */
  {
    errno = ENOENT;
    return -1;
  }
  return 0;
}
