/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
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

  if (_USE_LFN)
  {
    r.x.flags = 1;  /* Always set CF before calling a 0x71NN function. */
    r.x.ax = 0x713a;
  }
  else
    r.h.ah = 0x3a;
  r.x.ds = __tb_segment;
  r.x.dx = __tb_offset;
  _put_path(real_dir);
  __dpmi_int(0x21, &r);

  if ((r.x.flags & 1) || (r.x.ax == 0x7100))
  {
    /*  Never assume that the complete LFN API is implemented,
        so check that AX != 0x7100.  E.G.: MSDOS 6.22 and DOSLFN 0.40.
        If not supported fail.  */
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  return 0;
}
