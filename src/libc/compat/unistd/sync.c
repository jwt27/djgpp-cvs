/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <io.h>

int
sync(void)
{
  int i;
  /* Update files with handles 0 thru 254 (incl).  */
  for (i = 0; i < 255; i++)
    fsync (i);
  _flush_disk_cache ();
  return 0;
}
