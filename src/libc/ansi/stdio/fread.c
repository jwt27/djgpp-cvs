/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <libc/file.h>

size_t
fread(void *vptr, size_t size, size_t count, FILE *iop)
{
  char *ptr = (char *)vptr;
  int s;
  int c;

  s = size * count;
  if(!__is_text_file(iop))
  {
    while (s > 0) {
      if (iop->_cnt < s) {
	if (iop->_cnt > 0) {
	  memcpy(ptr, iop->_ptr, iop->_cnt);
	  ptr += iop->_cnt;
	  s -= iop->_cnt;
	}
	/*
	 * filbuf clobbers _cnt & _ptr,
	 * so don't waste time setting them.
	 */
	if ((c = _filbuf(iop)) == EOF)
	  break;
	*ptr++ = c;
	s--;
      }
      if (iop->_cnt >= s) {
	memcpy(ptr, iop->_ptr, s);
	iop->_ptr += s;
	iop->_cnt -= s;
	return count;
      }
    }
  }
  else
  {
    while (s > 0) {
      if (iop->_cnt < s) {
	while (iop->_cnt > 0) {
	  if ((c = *iop->_ptr++) != '\r')
	  {
	    *ptr++ = c;
	    s--;
	  }
	  iop->_cnt--;
	}
	if ((c = _filbuf(iop)) == EOF)
	  break;
	if (c != '\r')
	{
	  *ptr++ = c;
	  s--;
	}
      }
      if (iop->_cnt >= s) {
	while (s > 0 && iop->_cnt > 0) {
	  if ((c = *iop->_ptr++) != '\r')
	  {
	    *ptr++ = c;
	    s--;
	  }
	  iop->_cnt--;
	}
      }
    } /* end while */
  }
  return size != 0 ? count - ((s + size - 1) / size) : 0;
}
