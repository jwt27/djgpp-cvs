/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <go32.h>
#include <dpmi.h>
#include <dir.h>
#include <libc/dosio.h>

int
findfirst(const char *pathname, struct ffblk *ffblk, int attrib)
{
  __dpmi_regs r;
  int pathlen;

  if (pathname == 0 || ffblk == 0)
  {
    errno = EACCES;
    return -1;
  }

  pathlen = strlen(pathname) + 1;

  _put_path(pathname);
  if(_USE_LFN) {

    /* si = 1 indicates DOS style dates, 0 means Win32 type dates.
       DOS style dates are broken in some Win95 betas, build for either.
       Release works with DOS date, it's faster, so use it. */
    #define USEDOSDATE 1
    #if USEDOSDATE == 1
      #define _Win32_to_DOS (long)
    #else
      extern long _Win32_to_DOS(long long WinTime);
    #endif

    r.x.ax = 0x714e;
    r.x.cx = attrib;
    r.x.dx = __tb_offset;
    r.x.ds = __tb_segment;
    r.x.di = __tb_offset + pathlen;
    r.x.es = r.x.ds;
    r.x.si = USEDOSDATE;
    __dpmi_int(0x21, &r);
    if(!(r.x.flags & 1)) {
      struct ffblklfn ffblk32;
      /* Recover results */
      dosmemget(__tb+pathlen, sizeof(struct ffblklfn), &ffblk32);

      ffblk->ff_attrib = (char)ffblk32.fd_attrib;
      *(long *)(&ffblk->ff_ftime) = _Win32_to_DOS(ffblk32.fd_mtime);
      ffblk->ff_fsize = ffblk32.fd_size;
      strcpy(ffblk->ff_name, ffblk32.fd_longname);
      strcpy(ffblk->lfn_magic, "LFN32");

      /* If no wildcards, close the handle */
      if(!strchr(pathname,'*') && !strchr(pathname,'?')) {
        r.x.bx = r.x.ax;
        r.x.ax = 0x71a1;
        __dpmi_int(0x21, &r);
        r.x.ax = 0;
      }
        
      ffblk->lfn_handle = r.x.ax;
      *(long *)(&ffblk->lfn_ctime) = _Win32_to_DOS(ffblk32.fd_ctime);
      *(long *)(&ffblk->lfn_atime) = _Win32_to_DOS(ffblk32.fd_atime);

      return 0;
    }
  } else {

    #define _sizeof_dos_ffblk 44
    /* There will be a _sizeof_dos_ffblk character return value from findfirst 
       in the DTA.  Put the file name before this.  First set the DTA to be 
       transfer buffer. */

    r.x.dx = __tb_offset + pathlen;
    r.x.ds = __tb_segment;
    r.h.ah = 0x1a;
    __dpmi_int(0x21, &r);

    r.h.ah = 0x4e;
    r.x.dx = __tb_offset;
    r.x.ds = __tb_segment;
    r.x.cx = attrib;
    __dpmi_int(0x21, &r);
    if(!(r.x.flags & 1)) {
      /* Recover results */
      dosmemget(__tb+pathlen, _sizeof_dos_ffblk, ffblk);
      return 0;
    }
  }

  errno = __doserr_to_errno(r.x.ax);
  return errno;
}
