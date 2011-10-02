/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/symlink.h>
#include <stdarg.h>
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <dpmi.h>
#include <go32.h>
#include <libc/dosio.h>
#include <sys/fsext.h>
#include <libc/fsexthlp.h>

int
remove(const char *fn)
{
  __dpmi_regs r;
  int attr;
  int directory_p;
  int use_lfn = _USE_LFN;
  char real_name[FILENAME_MAX];
  int rv;

  /* Handle symlinks */
  if (!__solve_dir_symlinks(fn, real_name))
    return -1;

  /* see if a file system extension wants to handle this */
  if (__FSEXT_call_open_handlers_wrapper(__FSEXT_unlink, &rv, real_name))
    return rv;

  /* Get the file attribute byte.  */
  attr = _chmod(real_name, 0);

  /* The above _chmod will return -1 if the file
     does not exist.  If it doesn't, don't bother
     doing anything else, return an error, and
     set errno properly. */
  if (attr == -1)
  {
    errno = ENOENT;
    return(-1);
  }

  directory_p = attr & 0x10;
 
  /* Now, make the file writable.  We must reset Vol, Dir, Sys and Hidden bits 
     in addition to the Read-Only bit, or else 214301 will fail.  */
  _chmod(real_name, 1, attr & 0xffe0);

  /* Now delete it.  Note, _chmod leaves dir name in transfer buffer. */
  if (directory_p)
    r.h.ah = 0x3a;		/* DOS Remove Directory function */
  else
    r.h.ah = 0x41;		/* DOS Remove File function */
  if (use_lfn)
  {
    r.x.flags = 1;		/* Always set CF before calling a 0x71NN function. */
    r.h.al = r.h.ah;
    r.h.ah = 0x71;
    r.x.si = 0;			/* No Wildcards */
  }
  r.x.cx = 0;			/* Fix for ROM-DOS */
  r.x.dx = __tb_offset;
  r.x.ds = __tb_segment;
  __dpmi_int(0x21, &r);
  if ((r.x.flags & 1) || (r.x.ax == 0x7100))
  {
    /*  Never assume that the complete LFN API is implemented,
        so check that AX != 0x7100.  E.G.: MSDOS 6.22 and DOSLFN 0.40.
        If not supported fall back on SFN API 0x3A or 0x41.  */

    /* We failed.  Leave the things as we've found them.  */
    int e = __doserr_to_errno(r.x.ax);

    /* We know the file exists, so ENOENT at this point means a bug.
       Since write-protected floppies are the most probable cause,
       return EACCES instead.  */
    if (e == ENOENT)
      e = EACCES;
 
    _chmod(real_name, 1, attr & 0xffe7);
    errno = e;
    return -1;
  }
  return 0;
}
