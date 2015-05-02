/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

ldiv_t
ldiv(long num, long denom)
{
  ldiv_t r;

  r.rem = num % denom;
  r.quot = num / denom;

  return r;
}
