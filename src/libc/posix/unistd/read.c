/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <io.h>
#include <errno.h>

#include <libc/dosio.h>
#include <libc/ttyprvt.h>
#include <libc/fd_props.h>

ssize_t
read(int handle, void* buffer, size_t count)
{
  ssize_t ngot;

  /* termios special hook */
  if (__libc_read_termios_hook != NULL)
    {
      ssize_t rv;
      if (__libc_read_termios_hook(handle, buffer, count, &rv) != 0)
        return rv;
    }

  /* Directory? If so, fail, which indicates that the caller should use
   * readdir() instead. */
  if (__get_fd_flags(handle) & FILE_DESC_DIRECTORY)
  {
    errno = EISDIR;
    return -1;
  }

  ngot = _read(handle, buffer, count);
  if(ngot <= 0)
    return ngot;
  if (__file_handle_modes[handle] & O_TEXT)
  {
    /* check for Ctrl-Z EOF character */
    int i;
    for (i=0; i<ngot; i++)
      if (((char *)buffer)[i] == 0x1a) /* Ctrl-Z */
      {
	lseek(handle, i-ngot, SEEK_CUR); /* back up to make ^Z first char read next time */
	ngot = i;
	break;
      }

    if (ngot > 0)
    {
      /* convert CR/LF to NL */
      ngot = crlf2nl(buffer, ngot);
      if(!ngot)
	return read(handle, buffer, count);		/* if all CR's read more data */
    }
  }
  return ngot;
}
