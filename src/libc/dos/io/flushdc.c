#include <libc/stubs.h>
#include <fcntl.h>	/* for _USE_LFN */
#include <io.h>		/* for the prototype of `_flush_disk_cache' */
#include <dir.h>	/* for `getdisk' */
#include <dpmi.h>	/* for `__dpmi_int' and friends */

/* Try to cause the disk cache to write the cached data to disk(s).  */
void
_flush_disk_cache (void)
{
  __dpmi_regs r;
  int drv = getdisk ();

  if (_USE_LFN)
    {
      /* Windows 95 have special function to do what we want.  */
      /* FIXME: What if LFN is supported by a platform other than W95?  */
      r.x.ax = 0x710d;
      r.x.cx = 1;	/* flush buffers and cache, reset drive */
      r.x.dx = drv + 1;
      __dpmi_int (0x21, &r);
      /* According to docs (Interrupt list), this doesn't return
	 any error codes (??).  */
    }
  else
    {
      /* The BIOS Disk Reset function causes most DOS caches to flush.  */
      r.x.ax = 0;
      /* Hard disks should have 7th bit set.  */
      /* FIXME: The mapping between DOS drive numbers and BIOS
	 drives is ignored.  The assumption is that Reset function
	 on ANY hard disk causes the cache to flush its buffers.  */
      r.x.dx = drv > 2 ? ((drv - 2) | 0x80) : drv;
      __dpmi_int (0x13, &r);
    }
}
