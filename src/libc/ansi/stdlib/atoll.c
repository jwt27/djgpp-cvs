/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

long long int
atoll(const char *str)
{
  return strtoll(str, 0, 10);
}
