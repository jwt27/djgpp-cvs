/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <ctype.h>
#include <inlines/ctype.ha>

int tolower(int c)
{
  return __tolower(c);
}

int __attribute__((alias("tolower"))) _tolower(int c);
