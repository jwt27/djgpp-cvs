/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <dpmi.h>
#include <go32.h>
#include <crt0.h>
#include <libc/farptrgs.h>
#include <libc/dosio.h>

char *
__getcwd(char *buf, size_t size)
{
  char *bp;
  __dpmi_regs r;
  size_t needed_length;
  int c;
  unsigned use_lfn = _USE_LFN;
  int preserve_case = _preserve_fncase();
  char *name_start;

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
  if(use_lfn)
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

  /* switch FOO\BAR to foo/bar, downcase where appropriate */
  buf[1] = ':';
  buf[2] = '/';
  for (bp = buf+3, name_start = bp - 1; *name_start; bp++)
  {
    char long_name[FILENAME_MAX], short_name[13];

    if (*bp == '\\')
      *bp = '/';
    if (!preserve_case && (*bp == '/' || *bp == '\0'))
    {
      memcpy(long_name, name_start+1, bp - name_start - 1);
      long_name[bp - name_start - 1] = '\0';
      if (!strcmp(_lfn_gen_short_fname(long_name, short_name), long_name))
      {
	while (++name_start < bp)
	  if (*name_start >= 'A' && *name_start <= 'Z')
	    *name_start += 'a' - 'A';
      }
      else
	name_start = bp;
    }
    else if (*bp == '\0')
      break;
  }

  /* get current drive */
  r.h.ah = 0x19;
  __dpmi_int(0x21, &r);

  buf[0] = r.h.al + (r.h.al < 26 ? 'a' : 'A');

  return buf;
}

#ifdef TEST

int main(void)
{
  printf (getcwd ((char *)0, FILENAME_MAX + 10));
  return 0;
}

#endif
