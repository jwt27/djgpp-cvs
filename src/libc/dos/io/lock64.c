/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <io.h>
#include <errno.h>
#include <libc/dosio.h>

int
lock64(int fd, long long offset, long long length)
{
  int ret = _dos_lk64(fd, offset, length);
#if 0 /* __doserr_to_errno does the same.  */
  /* change return code because Borland does it this way.  */
  if (ret == 0x21)
    ret = EACCES;
#endif
  if (ret != 0)
  {
    errno = __doserr_to_errno(ret);
    return -1;
  }
  else
    return 0;
}
