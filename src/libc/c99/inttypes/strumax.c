/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <inttypes.h>
#include <stdlib.h>

uintmax_t
strtoumax (const char *nptr, char **endptr, int base)
{
  return(strtoull(nptr, endptr, base));
}
