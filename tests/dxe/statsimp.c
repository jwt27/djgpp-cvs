/*
 * Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
 *
 * Usage of this library is not restricted in any way.  No warranty.
 *
 * This sample demonstrates how to use static linkage against
 * dynamically-loadable modules. This requires no special programming
 * tricks at all, and requires just a special linking mode.
 */


#include <stdio.h>

extern int my_strlen (const char *);

int main ()
{
  printf ("my_strlen (\"abcde\") = %d\n", my_strlen ("abcde"));
  return 0;
}
