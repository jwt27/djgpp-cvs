/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <unistd.h>
#include <libc/file.h>
#include <fcntl.h>
#include <libc/dosio.h>

long
ftell(FILE *f)
{
  long tres;
  int adjust=0;
  int idx;

  if (f->_cnt < 0)
    f->_cnt = 0;
  if (f->_flag&_IOREAD)
  {
    if (__file_handle_modes[f->_file] & O_TEXT) /* if a text file */
    {
      if (f->_cnt && f->_ptr != f->_base)
      {
	char *cp;
	adjust = - f->_bufsiz + (f->_ptr-f->_base);
	for (cp=f->_base; cp < f->_ptr; cp++) /* for every char in buf */
	  if (*cp == '\n')	/* if it's LF */
	    adjust++;		/* there was a CR also */
      }
    }
    else
      adjust = - f->_cnt;
  }
  else if (f->_flag&(_IOWRT|_IORW))
  {
    adjust = 0;
    if (f->_flag&_IOWRT && f->_base && (f->_flag&_IONBF)==0)
    {
      adjust = f->_ptr - f->_base;
      if (__file_handle_modes[f->_file] & O_TEXT)
	for (idx=0; idx<adjust; idx++)
	  if (f->_base[idx] == '\n')
	    adjust++;
    }
  }
  else
    return -1;
  tres = lseek(fileno(f), 0L, 1);
  if (tres<0)
    return tres;
  tres += adjust;
  return tres;
}
