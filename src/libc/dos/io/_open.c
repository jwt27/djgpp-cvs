/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <io.h>
#include <dos.h>
#include <libc/dosio.h>
#include <sys/fsext.h>
#include <libc/fsexthlp.h>

int
_open(const char* filename, int oflag)
{
  __dpmi_regs r;
  int rv;
  int use_lfn = _USE_LFN;

  if (filename == 0)
  {
    errno = EINVAL;
    return -1;
  }

  if (__FSEXT_call_open_handlers_wrapper(__FSEXT_open, &rv, filename, oflag))
    return rv;

  if (use_lfn && _os_trueversion == 0x532)
  {
    /* Windows 2000 or XP; or NT with LFN TSR.  Windows 2000 behaves
       badly when using IOCTL and write-truncate calls on LFN handles.
       We convert the long name to a short name and open existing files
       via short name.  New files use LFN, but we know they aren't
       character devices. */
    r.x.flags = 1;			/* Always set CF before calling a 0x71NN function. */
    r.x.ax = 0x7160;
    r.x.cx = 1;				/* Get short name equivalent */
    r.x.ds = __tb_segment;
    r.x.si = __tb_offset;		/* Long name to convert - putpath */
    r.x.es = __tb_segment;
    r.x.di = __tb_offset + _put_path(filename);	/* Short name destination */
    __dpmi_int(0x21, &r);
    if (!(r.x.flags & 1))		/* Get short name success */
    {
      r.x.ax = 0x6c00;
      r.x.bx = (oflag & 0xff);
      r.x.dx = 1;			/* Open existing file */
      r.x.si = r.x.di;
      goto do_open;
    }
    else
    {
      /* Short name get failed, file doesn't exist or is device (same error) */
      r.x.flags = 1;			/* Always set CF before calling a 0x71NN function. */
      r.x.ax = 0x7143;			/* Get attributes */
      r.h.bl = 0;
      r.x.dx = __tb_offset;		/* Original long name */
      __dpmi_int(0x21, &r);		/* This is same as lfn _chmod */
      if (!(r.x.flags & 1))		/* Name exists, probably device */
      {
        r.x.ax = 0x6c00;
        r.x.bx = (oflag & 0xff);
        r.x.dx = 1;                     /* Open existing file */
        r.x.si = __tb_offset;		/* Treat original name as short */
        r.x.cx = 0;
        __dpmi_int(0x21, &r);
        if (!(r.x.flags & 1))		/* Success! */
        {
          goto do_hset;
        }
	/* Fail on short name open after _chmod said OK.
	   Device with directory?  We should re-try with LFN.
	   Permission?  Readonly file?  We should quit.
	   Let it fall through to the LFN open which should succeed.  */
      }
    }
  }
  if (use_lfn)
  {
    r.x.flags = 1;		/* Always set CF before calling a 0x71NN function. */
    r.x.ax = 0x716c;
    r.x.bx = (oflag & 0xff);
    /* The FAT32 bit should _not_ be set on Windows 2000, because
       that bit fails function 716Ch on W2K.  The test below is
       based on the assumption that W2K returns DOS version 5.  */
    if (7 <= _osmajor && _osmajor < 10)
    {
      r.x.bx |= 0x1000; /* 0x1000 is FAT32 extended size. */
    }
    r.x.dx = 1;			/* Open existing file */
    r.x.si = __tb_offset;
  }
  else
  {
    if (7 <= _osmajor && _osmajor < 10)
    {
      r.x.ax = 0x6c00;
      r.x.bx = (oflag & 0xff) | 0x1000; /* 0x1000 is FAT32 extended size. */
      /* FAT32 extended size flag doesn't help on WINDOZE 4.1 (98). It
	 seems it has a bug which only lets you create these big files
	 if LFN is enabled. */
      r.x.dx = 1;                        /* Open existing file */
      r.x.si = __tb_offset;
    }
    else
    {
      r.h.ah = 0x3d;
      r.h.al = oflag;
      r.x.dx = __tb_offset;
    }
  }
  r.x.ds = __tb_segment;
  _put_path(filename);
do_open:
  r.x.cx = 0;
  __dpmi_int(0x21, &r);

  if (r.x.ax == 0x7100)
  {
    /*  Never assume that the complete LFN API is implemented,
        so check that AX != 0x7100.  E.G.: MSDOS 6.22 and DOSLFN 0.40.
        If not supported fall back on SFN API 0x3D.  */
    use_lfn = 0;
    r.h.ah = 0x3d;
    r.h.al = oflag;
    r.x.dx = __tb_offset;
    goto do_open;
  }
  else if (r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
do_hset:
  __file_handle_set(r.x.ax, O_BINARY);
  return r.x.ax;
}
