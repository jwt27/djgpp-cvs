/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <time.h>
#include <dpmi.h>

int
settimeofday(struct timeval *_tp, ...)
{
  __dpmi_regs r;
  struct tm tm;

  tm = *localtime(&(_tp->tv_sec));

  if (tm.tm_year < 80)
    return -1;

  r.h.ah = 0x2b;
  r.x.cx = tm.tm_year + 1900;
  r.h.dh = tm.tm_mon + 1;
  r.h.dl = tm.tm_mday;
  __dpmi_int(0x21, &r);

  r.h.ah = 0x2d;
  r.h.ch = tm.tm_hour;
  r.h.cl = tm.tm_min;
  r.h.dh = tm.tm_sec;
  r.h.dl = _tp->tv_usec / 10000;
  __dpmi_int(0x21, &r);

  return 0;
}
