/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>

#include <libc/dosio.h>
#include <libc/bss.h>

static char *sbuf = 0;
static size_t sbuflen = 0;

static int write_count = -1;

ssize_t
write(int handle, const void* buffer, size_t count)
{
  if (count == 0)
    return 0; /* POSIX.1 requires this */

  if(__file_handle_modes[handle] & O_BINARY)
    return _write(handle, buffer, count);
  else
  {
    int nput, ocount;
    unsigned bufp=0, sbufp=0, crcnt=0;
    const char *buf;
    ocount = count;

    /* Force reinitialization in restarted programs (emacs).  */
    if (write_count != __bss_count)
    {
      write_count = __bss_count;
      sbuf = 0;
      sbuflen = 0;
    }

    if(sbuflen < 2*count)
    {
      if(sbuf != 0)
	free(sbuf);
      sbuflen = 2*count;
      sbuf = (char *)malloc(sbuflen);
      if (sbuf == 0)
      {
        errno = ENOMEM;
        return -1;
      }
    }
    buf = buffer;
    while (ocount--)
    {
      if(buf[bufp] == 10)
      {
	crcnt++;
	sbuf[sbufp++] = 13;
      }
      sbuf[sbufp++] = buf[bufp++];
    }
    ocount = count;
    count += crcnt;
    buffer = sbuf;
    nput = _write(handle, buffer, count);
    if (nput < ocount)
      return nput;		/* Maybe disk full? */
    return ocount;		/* But don't return count with CR's (TC) */
  }
}
