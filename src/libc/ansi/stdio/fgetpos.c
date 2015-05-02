/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <errno.h>

int
fgetpos(FILE *stream, fpos_t *pos)
{
  long ret;

  if (stream && pos)
  {
    ret = ftell(stream);
    if (ret != -1L)
      {
	*pos = (fpos_t)ret;
	return 0;
      }
    else
      {
	/* ftell will have set errno appropriately. */
	return -1;
      }
  }
  errno = EFAULT;
  return 1;
}
