/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <fcntl.h>
#include <go32.h>
#include <dpmi.h>
#include <io.h>
#include <errno.h>
#include <libc/farptrgs.h>
#include <sys/fsext.h>
#include <libc/dosio.h>
#include <libc/ttyprvt.h>

#define tblen _go32_info_block.size_of_transfer_buffer

int (*__libc_write_termios_hook)(int handle, const void *buffer, size_t count,
				 ssize_t *rv) = NULL;

ssize_t
write(int handle, const void* buffer, size_t count)
{
  const char *buf = (const char *)buffer;
  int bytes_in_tb = 0;
  int offset_into_buf = 0;
  __dpmi_regs r;

  ssize_t rv;
  __FSEXT_Function *func = __FSEXT_get_function(handle);

  /* termios special hook */
  if (__libc_write_termios_hook != NULL)
      if (__libc_write_termios_hook(handle, buffer, count, &rv) != 0)
        return rv;

  if (count == 0)
    return 0; /* POSIX.1 requires this */

  if(__file_handle_modes[handle] & O_BINARY)
    return _write(handle, buf, count);

  /* Let's handle FSEXT_write ! */
  if(func &&				/* if handler is installed, ...*/
     func(__FSEXT_write, &rv, &handle)) /* ... call extension ... */
      return rv;			/* ... and exit if handled. */

  while (offset_into_buf < count)
  {
    _farsetsel(_dos_ds);
    while (bytes_in_tb < tblen && offset_into_buf < count)
    {
      if (buf[offset_into_buf] == '\n')
      {
	if (bytes_in_tb == tblen - 1)
	  break; /* can't fit two more */
	_farnspokeb(__tb + bytes_in_tb, '\r');
	bytes_in_tb++;
      }
      _farnspokeb(__tb + bytes_in_tb, buf[offset_into_buf]);
      bytes_in_tb++;
      offset_into_buf++;
    }

    /* we now have a transfer buf stuffed with data; write it out */
    r.x.ax = 0x4000;
    r.x.bx = handle;
    r.x.cx = bytes_in_tb;
    r.x.dx = __tb & 15;
    r.x.ds = __tb / 16;
    __dpmi_int(0x21, &r);
    if (r.x.flags & 1)
    {
      errno = __doserr_to_errno(r.x.ax);
      return -1;
    }

    if (r.x.ax < bytes_in_tb) /* disk full? */
    {
      offset_into_buf += r.x.ax;
      errno = ENOSPC;
      return count - offset_into_buf;
    }

    bytes_in_tb = 0;
  }

  return count;	/* Don't return count with CR's (TC) */
}
