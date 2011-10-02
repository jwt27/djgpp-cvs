/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <io.h>
#include <dos.h>
#include <unistd.h>
#include <libc/dosio.h>
#include <sys/fsext.h>
#include <libc/fsexthlp.h>

int
_creatnew(const char* filename, int attrib, int flags)
{
  __dpmi_regs r;
  int rv;
  unsigned use_lfn = _USE_LFN;

  if (filename == 0)
  {
    errno = EINVAL;
    return -1;
  }

  if (__FSEXT_call_open_handlers_wrapper(__FSEXT_creat, &rv,
					 filename, attrib, flags))
    return rv;

  _put_path(filename);
  r.x.bx =
    0x2002 | (flags & 0xfff0);	/* r/w, no Int 24h, use caller-defined flags */
  r.x.dx = 0x0010;		/* Create, fail if exists */
  r.x.si = __tb_offset;
  if (use_lfn)
  {
    if (7 <= _osmajor && _osmajor < 10)
    {
      r.x.bx |= 0x1000; 	/* FAT32 extended size. */
    }
    r.x.flags = 1; 		/* Always set CF before calling a 0x71NN function. */
    r.x.ax = 0x716c;
  }
  else
  {
    if (7 <= _osmajor && _osmajor < 10)
    {
      r.x.bx |= 0x1000; 	/* FAT32 extended size. */
      /* FAT32 extended size flag doesn't help on WINDOZE 4.1 (98). It
	 seems it has a bug which only lets you create these big files
	 if LFN is enabled. */
    }

    if (_osmajor > 3)
      r.x.ax = 0x6c00;
    else
    {
      r.h.ah = 0x5b;
      r.x.bx = 0;		/* lose support for fancy flags in DOS 3.x */
      r.x.dx = __tb_offset;
      r.x.si = 0;
    }
  }
do_create:
  r.x.cx = attrib & 0xffff;
  r.x.ds = __tb_segment;
  __dpmi_int(0x21, &r);

  if (r.x.ax == 0x7100)
  {
    /*  Never assume that the complete LFN API is implemented,
        so check that AX != 0x7100.  E.G.: MSDOS 6.22 and DOSLFN 0.40.
        If not supported fall back on SFN API 0x5B.  */
    use_lfn = 0;
    r.h.ah = 0x5b;
    r.x.bx = 0;		/* lose support for fancy flags in DOS 3.x */
    r.x.dx = __tb_offset;
    r.x.si = 0;
    goto do_create;
  }
  else if (r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  if (use_lfn && _os_trueversion == 0x532)
  {
    /* Windows 2000 or XP; or NT with LFN TSR.  Windows 2000 behaves
       badly when using IOCTL and write-truncate calls on LFN handles.
       We close the long name file and re-open it with _open.c (short)
       to work around the bugs. */
    rv = _open(filename, flags | 2);	/* 2 is a read/write flag */
    if (rv != -1)
    {
      dup2(rv, r.x.ax);	/* Re-open failure, continue with LFN handle */
      _close(rv);	/* Close ax, put handle in first position (bugs) */
    }
  }
  __file_handle_set(r.x.ax, O_BINARY);
  return r.x.ax;
}
