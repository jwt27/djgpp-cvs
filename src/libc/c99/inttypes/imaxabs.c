/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <inttypes.h>

intmax_t
imaxabs (intmax_t j)
{
  if (j < 0)
    return(-j);
  return(j);
}
