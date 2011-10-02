/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <fcntl.h>	/* for _USE_LFN */
#include <io.h>		/* for the prototype of `_flush_disk_cache' */
#include <dir.h>	/* for `getdisk' */
#include <dpmi.h>	/* for `__dpmi_int' and friends */

/* Try to cause the disk cache to write the cached data to disk(s).  */
void
_flush_disk_cache(void)
{
  __dpmi_regs r;
  int drv = getdisk();

  if (_USE_LFN)
  {
    /* Windows 95 have special function to do what we want.  */
    /* FIXME: What if LFN is supported by a platform other than W95?  */
    r.x.flags = 1;  /* Always set CF before calling a 0x71XX function. */
    r.x.ax = 0x710d;
    r.x.cx = 1;      /* flush buffers and cache, reset drive */
    r.x.dx = drv + 1;
    __dpmi_int (0x21, &r);
    if ((r.x.flags & 1) || (r.x.ax == 0x7100))
    {
      /*  Never assume that the complete LFN API is implemented,
          so check that AX != 0x7100.  E.G.: MSDOS 6.22 and DOSLFN 0.40.
          If not supported fall back on SFN API.  */
      goto do_BIOS_DISK_RESET;
    }
    return;
  }

do_BIOS_DISK_RESET:
  /* The BIOS Disk Reset function causes most DOS caches to flush.  */
  r.x.ax = 0;
  /* Hard disks should have 7th bit set.  */
  /* FIXME: The mapping between DOS drive numbers and BIOS
     drives is ignored.  The assumption is that Reset function
     on ANY hard disk causes the cache to flush its buffers.  */
  r.x.dx = drv > 2 ? ((drv - 2) | 0x80) : drv;
  __dpmi_int(0x13, &r);
}
