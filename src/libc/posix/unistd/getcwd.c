/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <dpmi.h>
#include <go32.h>
#include <libc/farptrgs.h>
#include <libc/dosio.h>

char *
getcwd(char *buf, size_t size)
{
  char *bp;
  __dpmi_regs r;
  int needed_length, c;

  if (!size)
  {
    errno = EINVAL;
    return 0;
  }
  if (!buf)
  {
    buf = (char *)malloc(size);
    if (!buf)
    {
      errno = ENOMEM;
      return 0;
    }
  }

  /* make sure we don't overrun the TB */
  if (size > _go32_info_block.size_of_transfer_buffer)
    size = _go32_info_block.size_of_transfer_buffer;

  /* get the path into the transfer buffer at least */
  if(_USE_LFN)
    r.x.ax = 0x7147;
  else
    r.h.ah = 0x47;
  r.h.dl = 0;
  r.x.si = __tb_offset;
  r.x.ds = __tb_segment;
  __dpmi_int(0x21, &r);

  /* current drive may be invalid (it happens) */
  if (r.x.flags & 1)
  {
    errno = __doserr_to_errno(r.x.ax);
    return 0;
  }

  /* path is ASCIIZ.  Scan it, filling in buf, watching for
     end of buffer. */
  _farsetsel(_dos_ds);
  needed_length = 0;
  while ((c = _farnspeekb(__tb+needed_length)))
  {
    if (needed_length+3 >= size)
    {
      errno = ERANGE;
      return 0;
    }
    buf[3+needed_length] = c;
    needed_length++;
  }
  buf[needed_length+3] = 0;

  /* switch FOO\BAR to foo/bar */
  for (bp = buf+3; *bp; bp++)
  {
    if (*bp == '\\')
      *bp = '/';
    if (*bp >= 'A' && *bp <= 'Z')
      *bp += 'a' - 'A';
  }

  /* get current drive */
  r.h.ah = 0x19;
  __dpmi_int(0x21, &r);

  buf[0] = r.h.al + 'a';
  buf[1] = ':';
  buf[2] = '/';

  return buf;
}
