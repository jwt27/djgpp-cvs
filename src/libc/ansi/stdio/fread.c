/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libc/file.h>
#include <libc/ttyprvt.h>

size_t
fread(void *vptr, size_t size, size_t count, FILE *f)
{
  char *ptr = (char *)vptr;
  int s;
  int c;

  /* grow if we know we're asking for a lot, even if it's in the
     buffer, since we'll probably read chunks this size for a while */
  while (size * count > f->_fillsize
         && f->_fillsize < f->_bufsiz)
  {
    if (f->_fillsize < 512)
      f->_fillsize = 512;
    f->_fillsize *= 2;
  }

  if (__libc_read_termios_hook
      && !(f->_flag & (_IOTERM | _IONTERM)))
  {
    /* first time we see this handle--see if termios hooked it */
    if (isatty(f->_file))
      f->_flag |= _IOTERM;
    else
      f->_flag |= _IONTERM;
  }

  s = size * count;
  if (!__is_text_file(f))
  {
    while (s > 0)
    {
      if (f->_cnt < s)
      {
        if (f->_cnt > 0)
        {
          memcpy(ptr, f->_ptr, (size_t)f->_cnt);
          ptr += f->_cnt;
          s -= f->_cnt;
        }
        /*
         * filbuf clobbers _cnt & _ptr,
         * so don't waste time setting them.
         */
        if ((c = _filbuf(f)) == EOF)
          break;
        *ptr++ = c;
        s--;
      }
      if (f->_cnt >= s)
      {
        memcpy(ptr, f->_ptr, (size_t)s);
        f->_ptr += s;
        f->_cnt -= s;
        return count;
      }
    }
  }
  else
  {
    while (s > 0)
    {
      if (f->_cnt < s)
      {
        while (f->_cnt > 0)
        {
          if ((c = *f->_ptr++) != '\r')
          {
            *ptr++ = c;
            s--;
          }
          f->_cnt--;
        }
        if ((c = _filbuf(f)) == EOF)
          break;
        if (c != '\r')
        {
          *ptr++ = c;
          s--;
        }
      }
      if (f->_cnt >= s)
      {
        while (s > 0 && f->_cnt > 0)
        {
          if ((c = *f->_ptr++) != '\r')
          {
            *ptr++ = c;
            s--;
          }
          f->_cnt--;
        }
      }
    } /* end while */
  }
  return size != 0 ? count - ((s + size - 1) / size) : 0;
}
