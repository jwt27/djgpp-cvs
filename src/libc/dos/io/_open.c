/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <io.h>
#include <dos.h>
#include <libc/dosio.h>
#include <sys/fsext.h>

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

  if (__FSEXT_call_open_handlers(__FSEXT_open, &rv, &filename))
    return rv;

  if(use_lfn) {
    r.x.ax = 0x716c;
    r.x.bx = (oflag & 0xff) | 0x1000; /* 0x1000 is FAT32 extended size. */
    r.x.dx = 1;			/* Open existing file */
    r.x.si = __tb_offset;
  } else if(7 <= (_get_dos_version(1) >> 8)) {
    r.x.ax = 0x6c00;
    r.x.bx = (oflag & 0xff) | 0x1000; /* 0x1000 is FAT32 extended size. */
    /* FAT32 extended size flag doesn't help on WINDOZE 4.1 (98). It
       seems it has a bug which only lets you create these big files
       if LFN is enabled. */
    r.x.dx = 1;                        /* Open existing file */
    r.x.si = __tb_offset;
  } else {
    r.h.ah = 0x3d;
    r.h.al = oflag;
    r.x.dx = __tb_offset;
  }
  r.x.cx = 0;
  r.x.ds = __tb_segment;
  _put_path(filename);
  __dpmi_int(0x21, &r);
  if(r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return -1;
  }
  __file_handle_set(r.x.ax, O_BINARY);
  return r.x.ax;
}
