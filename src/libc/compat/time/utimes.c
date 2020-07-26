/* Copyright (C) 2020 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <sys/time.h>
#include <utime.h>

int
utimes(const char *file, const struct timeval tvp[2])
{
  struct utimbuf utb;
  utb.actime = tvp[0].tv_sec;
  utb.modtime = tvp[1].tv_sec;
  return utime(file, &utb);
}
