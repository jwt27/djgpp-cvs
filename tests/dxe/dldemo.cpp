/*
 * Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
 *
 * Usage of this library is not restricted in any way.  No warranty.
 *
 * This file demonstrates how to load DXE modules with undefined
 * external references. When you call dxeload() function you can
 * pass a pointer to a certain table which should contain all the
 * data required to resolve the undefined external references.
 * Also you can set a callback function which will be called as
 * a last resort to get the address of a function which's address
 * could not be found in the passed table.
 *
 * The second point is to demonstrate the re-export feature of DXE
 * library. By providing the RTLD_GLOBAL flag to dlopen() you make
 * all the symbols in the library automatically available to
 * subsequently loaded modules. In our example module "test3.dxe"
 * makes use of functions from "test2.dxe" as if they were usual
 * external C functions.
 *
 * Also note how the variable inside outer module is accessed.
 */


#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/dxe.h>

#define MODULE1 "test2.dxe"
#define MODULE2 "test3.dxe"

DXE_EXPORT_TABLE (syms)
  DXE_EXPORT (printf)
DXE_EXPORT_END

static int lastresort ()
{
  printf ("last resort function called!\n");
  return 0;
}

void *dxe_res (const char *symname)
{
  printf ("%s: undefined symbol in dynamic module\n", symname);
  return (void *)lastresort;
}

int main ()
{
  // Set the error callback function
  _dlsymresolver = dxe_res;

  // Register the symbols exported into dynamic modules
  dlregsym (syms);

  // Now try to load the first module
  void *h1 = dlopen (MODULE1, RTLD_GLOBAL);
  if (!h1)
  {
    printf (MODULE1 ": %s\n", dlerror ());
    exit (-1);
  }

  // And now try to load the second module
  void *h2 = dlopen (MODULE2, 0);
  if (!h2)
  {
    printf (MODULE2 ": %s\n", dlerror ());
    dlclose (h1);
    exit (-1);
  }

  // Touch the x_counter variable inside the data segment of the module
  int *x_counter = (int *)dlsym (h1, "_x_counter");
  if (x_counter)
    *x_counter += 50;

  // Allright, now call the main function from second module
  void (*test_reexp) () = (void (*)())dlsym (h2, "_test_reexp");
  if (test_reexp)
    test_reexp ();

  dlclose (h2);
  dlclose (h1);
  return 0;
}
