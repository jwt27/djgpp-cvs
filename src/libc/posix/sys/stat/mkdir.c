/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <errno.h>
#include <sys/stat.h>
#include <go32.h>
#include <dpmi.h>
#include <unistd.h>
#include <fcntl.h>
#include <libc/dosio.h>
 
int
mkdir(const char *dirname, mode_t mode)
{
  __dpmi_regs r;
  int use_lfn = _USE_LFN;

  _put_path(dirname);
 
  if(use_lfn)
    r.x.ax = 0x7139;
  else
    r.h.ah = 0x39;
  r.x.ds = __tb_segment;
  r.x.dx = __tb_offset;
  __dpmi_int(0x21, &r);
 
  if (r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    if (errno == EACCES)
    {
      /* see if the directory existed, in which case
	 we should return EEXIST - DJ */
      if (access(dirname, D_OK) == 0)
	errno = EEXIST;
    }
    return -1;
  }

  /* DOS ignores directory permissions, and mkdir is stub'd,
     so rather than stub chmod also, just skip it.   DJ */
/*  if (chmod(dirname, mode))
    return -1; */
  return 0;
}

