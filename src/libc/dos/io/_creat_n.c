/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <fcntl.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <io.h>
#include <dos.h>
#include <unistd.h>
#include <libc/dosio.h>
#include <sys/fsext.h>

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

  if (__FSEXT_call_open_handlers(__FSEXT_creat, &rv, &filename))
    return rv;

  _put_path(filename);
  r.x.bx =
    0x2002 | (flags & 0xfff0);	/* r/w, no Int 24h, use caller-defined flags */
  r.x.dx = 0x0010;		/* Create, fail if exists */
  r.x.si = __tb_offset;
  if(use_lfn)
  {
    if (7 <= _osmajor && _osmajor < 10)
    {
      r.x.bx |= 0x1000; 	/* FAT32 extended size. */
    }
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
  r.x.cx = attrib & 0xffff;
  r.x.ds = __tb_segment;
  __dpmi_int(0x21, &r);
  if(r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  if(use_lfn && _os_trueversion == 0x532) {
    /* Windows 2000 or XP; or NT with LFN TSR.  Windows 2000 behaves
       badly when using IOCTL and write-truncate calls on LFN handles.
       We close the long name file and re-open it with _open.c (short)
       to work around the bugs. */
    rv = _open(filename, flags | 2);	/* 2 is a read/write flag */
    if(rv != -1) {	/* Re-open failure, continue with LFN handle */
      dup2(rv, r.x.ax);	/* Close ax, put handle in first position (bugs) */
      _close(rv);
    }
  }
  __file_handle_set(r.x.ax, O_BINARY);
  return r.x.ax;
}
