/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <time.h>

double
difftime(time_t time1, time_t time0)
{
  return (double)time1-(double)time0;
}
