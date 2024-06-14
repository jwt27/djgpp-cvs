/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <ctype.h>
#include <inlines/ctype.ha>

int toupper(int c)
{
  return __toupper(c);
}

//int __attribute__((alias("toupper"))) _toupper(int c);
