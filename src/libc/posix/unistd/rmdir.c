/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/symlink.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <go32.h>
#include <dpmi.h>
#include <stdio.h>
#include <libc/dosio.h>

int
rmdir(const char *mydirname)
{
  __dpmi_regs r;
  char real_dir[FILENAME_MAX];

  if (!__solve_dir_symlinks(mydirname, real_dir))
    return -1;

  if(_USE_LFN)
    r.x.ax = 0x713a;
  else
    r.h.ah = 0x3a;
  r.x.ds = __tb_segment;
  r.x.dx = __tb_offset;
  _put_path(real_dir);
  __dpmi_int(0x21, &r);

  if (r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  return 0;
}
