/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <go32.h>
#include <libc/file.h>
#include <libc/stdiohk.h>
#include <io.h>

int
_filbuf(FILE *f)
{
  int size;
  char c;

  if (f->_flag & _IORW)
    f->_flag |= _IOREAD;

  if ((f->_flag&_IOREAD) == 0)
    return EOF;
  if (f->_flag&(_IOSTRG|_IOEOF))
    return EOF;
  f->_flag &= ~_IOUNGETC;

  if (f->_base==NULL && (f->_flag&_IONBF)==0) {
    size = _go32_info_block.size_of_transfer_buffer;
    if ((f->_base = malloc(size)) == NULL)
    {
      f->_flag |= _IONBF;
      f->_flag &= ~(_IOFBF|_IOLBF);
    }
    else
    {
      f->_flag |= _IOMYBUF;
      f->_bufsiz = size;
    }
  }

  if (f->_flag&_IONBF)
    f->_base = &c;

  if (f == stdin) {
    if (stdout->_flag&_IOLBF)
      fflush(stdout);
    if (stderr->_flag&_IOLBF)
      fflush(stderr);
  }
  f->_cnt = _read(fileno(f), f->_base,
		   f->_flag & _IONBF ? 1 : f->_bufsiz);
  if(__is_text_file(f) && f->_cnt>0)
  {
    /* truncate text file at Ctrl-Z */
    char *cz=memchr(f->_base, 0x1A, f->_cnt);
    if(cz)
    {
      int newcnt = cz - f->_base;
      lseek(fileno(f), -(f->_cnt - newcnt), SEEK_CUR);
      f->_cnt = newcnt;
    }
  }
  f->_ptr = f->_base;
  if (f->_flag & _IONBF)
    f->_base = NULL;
  if (--f->_cnt < 0) {
    if (f->_cnt == -1) {
      f->_flag |= _IOEOF;
      if (f->_flag & _IORW)
	f->_flag &= ~_IOREAD;
    } else
      f->_flag |= _IOERR;
    f->_cnt = 0;
    return EOF;
  }
  return *f->_ptr++ & 0377;
}
