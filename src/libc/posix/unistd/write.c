/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
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
#include <libc/fd_props.h>

#define tblen _go32_info_block.size_of_transfer_buffer

int (*__libc_write_termios_hook)(int handle, const void *buffer, size_t count,
				 ssize_t *rv) = NULL;

/* From _write.c. */
int _write_fill_seek_gap(int fd);
int _write_int(int fd, const char *buffer, size_t count);

ssize_t
write(int handle, const void* buffer, size_t count)
{
  const char *buf = (const char *)buffer;
  int bytes_in_tb = 0;
  int offset_into_buf = 0;
  int out;
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

  if (__has_fd_properties(handle)
      && (__fd_properties[handle]->flags & FILE_DESC_ZERO_FILL_EOF_GAP))
  {
    if (_write_fill_seek_gap(handle) < 0)
      return -1;
  }
  
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

    /* Write out the contents of the transfer buffer. */
    out = _write_int(handle, NULL, bytes_in_tb);
    if (out < 0)
      return -1;
    if (out < bytes_in_tb)
    {
      offset_into_buf += out;
      errno = ENOSPC;
      return count - offset_into_buf;
    }
    
    bytes_in_tb = 0;
  }

  return count;	/* Don't return count with CR's (TC) */
}
