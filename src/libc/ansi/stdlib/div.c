/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

div_t
div(int num, int denom)
{
  div_t r;

  r.rem = num % denom;
  r.quot = num / denom;

  return r;
}
