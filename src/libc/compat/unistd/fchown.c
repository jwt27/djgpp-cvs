/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdarg.h>
#include <sys/fsext.h>
#include <libc/fsexthlp.h>
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
    if (__FSEXT_func_wrapper(func, __FSEXT_fchown, &rv, fd, owner, group))
      return rv;
  }
  return (_get_dev_info(fd) == -1) ? -1 : 0;
}
