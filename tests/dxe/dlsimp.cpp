/*
 * Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
 *
 * Usage of this library is not restricted in any way.  No warranty.
 *
 * This sample demonstrates how to use simple DXEs (with no external
 * symbol references) using the Unix-like dlopen() API.
 */


#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define MODULE "test1.dxe"

int main ()
{
  // Try to load the module
  void *h = dlopen (MODULE, 0);
  if (!h)
  {
    printf (MODULE ": %s\n", dlerror ());
    exit (-1);
  }

  int (*my_strlen) (const char *) = (int (*) (const char *))dlsym (h, "_my_strlen");
  printf ("my_strlen (\"abcde\") = %d\n", my_strlen ("abcde"));

  dlclose (h);

  return 0;
}
