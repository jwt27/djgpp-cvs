/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <inttypes.h>
#include <stdlib.h>

intmax_t
strtoimax (const char *nptr, char **endptr, int base)
{
  return(strtoll(nptr, endptr, base));
}
