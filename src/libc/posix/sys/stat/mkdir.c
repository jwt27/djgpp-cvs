/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <errno.h>
#include <sys/stat.h>
#include <go32.h>
#include <dpmi.h>
#include <unistd.h>
#include <fcntl.h>
#include <io.h>
#include <dos.h>
#include <stdio.h>
#include <libc/dosio.h>
#include <libc/symlink.h>
 
int
mkdir(const char *mydirname, mode_t mode)
{
  __dpmi_regs r;
  int use_lfn = _USE_LFN;
  unsigned attr;
  char dir_name[FILENAME_MAX];

  if (!__solve_symlinks(mydirname, dir_name))
     return -1;
  
  _put_path(dir_name);
 
  if(use_lfn)
    r.x.ax = 0x7139;
  else if ((_osmajor > 7 && _osmajor < 10) /* OS/2 returns v10 and above */
	   || (_osmajor == 7 && _osminor >= 20))
  {
    /* DOS 7.20 (Windows 98) and later supports a new function with
       a maximum path length of 128 characters instead of 67.  This
       is important for deeply-nested directories.  */
    r.x.ax = 0x43ff;
    r.x.bp = 0x5053;
    r.h.cl = 0x39;
  }
  else
    r.h.ah = 0x39;
  r.x.ds = __tb_segment;
  r.x.dx = __tb_offset;
  __dpmi_int(0x21, &r);
 
  if (r.x.flags & 1)
  {
    int save_errno;
    save_errno = errno = __doserr_to_errno(r.x.ax);
    if (errno == EACCES)
    {
      /* see if the directory existed, in which case
	 we should return EEXIST - DJ */
      if (access(mydirname, D_OK) == 0)
	errno = EEXIST;
      else
	errno = save_errno;
    }
    return -1;
  }

  /* mkdir is stub'd, and we don't want to stub chmod also.  */
  attr = _chmod(dir_name, 0, 0);

  /* Must clear the directory and volume bits, otherwise 214301 fails.
     Unused bits left alone (some network redirectors use them).  Only
     care about read-only attribute.  */
  if (_chmod(dir_name, 1, (attr & 0xffe6) | ((mode & S_IWUSR) == 0)) == -1)
    return -1;
  return 0;
}
