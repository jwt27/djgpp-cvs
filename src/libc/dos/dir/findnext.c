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
findnext(struct ffblk *ffblk)
{
  __dpmi_regs r;

  if (ffblk == 0)
  {
    errno = EACCES;
    return -1;
  }

  if(_USE_LFN)
  {
    /* si = 1 indicates DOS style dates, 0 means Win32 type dates.
       DOS style dates are broken in some Win95 betas, build for either.
       Release works with DOS date, it's faster, so use it. */
    #define USEDOSDATE 1
    #if USEDOSDATE == 1
      #define _Win32_to_DOS (long)
    #else
      extern long _Win32_to_DOS(long long WinTime);
    #endif

    r.x.ax = 0x714f;
    r.x.bx = ffblk->lfn_handle;
    if(!r.x.bx)
    {
      errno = ENMFILE;
      return 1;
    }
    r.x.di = __tb_offset;
    r.x.es = __tb_segment;
    r.x.si = USEDOSDATE;

    __dpmi_int(0x21, &r);
    if (!(r.x.flags & 1))
    {
      struct ffblklfn ffblk32;
      /* Recover results */
      dosmemget(__tb, sizeof(struct ffblklfn), &ffblk32);

      ffblk->ff_attrib = (char)ffblk32.fd_attrib;
      *(long *)&ffblk->ff_ftime = _Win32_to_DOS(ffblk32.fd_mtime);
      ffblk->ff_fsize = ffblk32.fd_size;
      strcpy(ffblk->ff_name, ffblk32.fd_longname);
      *(long *)&ffblk->lfn_ctime = _Win32_to_DOS(ffblk32.fd_ctime);
      *(long *)&ffblk->lfn_atime = _Win32_to_DOS(ffblk32.fd_atime);

      return 0;
    }
    errno = __doserr_to_errno(r.x.ax);
    if (errno == ENMFILE)         /* call FindClose */
    {
      ffblk->lfn_handle = 0;
      r.x.ax = 0x71a1;
      __dpmi_int(0x21, &r);
      if(r.x.flags & 1)
      {
        errno = __doserr_to_errno(r.x.ax);
        return -1;
      }
      return 1;
    }
    return -1;
  }
  else
  {
    #define _sizeof_dos_ffblk 44
    /* The 43 character ff block must be put to the DTA, make the call, then
       recover the ff block. */

    r.x.dx = __tb_offset;
    r.x.ds = __tb_segment;
    r.h.ah = 0x1a;
    __dpmi_int(0x21, &r);

    dosmemput(ffblk, sizeof(struct ffblk), __tb);

    r.h.ah = 0x4f;
    __dpmi_int(0x21, &r);
    if(r.x.flags & 1)
    {
      errno = __doserr_to_errno(r.x.ax);
      return -1;
    }

    /* Recover results */
    dosmemget(__tb, _sizeof_dos_ffblk, ffblk);
    return 0;
  }
}
