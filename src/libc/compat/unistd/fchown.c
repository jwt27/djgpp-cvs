/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <sys/fsext.h>
#include <io.h>
#include <unistd.h>
 
/* MS-DOS couldn't care less about file ownerships, so we 
   at least check if given handle is valid. */
 
int fchown(int fd, uid_t owner, gid_t group)
{
  __FSEXT_Function * func = __FSEXT_get_function(fd);
  if (func)
  {
    int rv;
    if (func(__FSEXT_fchown, &rv, &fd))
      return rv;
  }
  return (_get_dev_info(fd) == -1) ? 1 : 0;
}
