/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <inttypes.h>

imaxdiv_t
imaxdiv (intmax_t numer, intmax_t denom)
{
  imaxdiv_t res;

  res.quot = numer / denom;
  res.rem  = numer % denom;

  return(res);
}
