/*
 * Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
 *
 * Usage of this library is not restricted in any way.  No warranty.
 *
 * This is a more complex static linking example.
 * Besides usual things (by the way, note that the DXE module
 * is written in C++ while this module is plain C) it shows how
 * to load-on-demand and unload-on-demand statically-linked modules.
 */


#include <stdio.h>
#include <sys/dxe.h>

extern int test_func ();
extern int doit ();

void extern_func ()
{
  printf ("extern_func ()\n");
}

DXE_EXPORT_TABLE_AUTO (syms)
  DXE_EXPORT (printf)
  DXE_EXPORT (extern_func)
DXE_EXPORT_END

DXE_DEMAND (TEST2);

int main ()
{
  /* Call the test_func() module entry */
  printf ("test_func: %d\n", test_func ());

  /* Call the doit() module function */
  printf ("doit: %d\n", doit ());
  printf ("doit: %d\n", doit ());

  /* Explicitly unload the module now */
  dlunload_TEST2 ();

  printf ("doit: %d\n", doit ());

  /* And now load it (this is a no-op since its already loaded by previous call) */
  dlload_TEST2 ();

  printf ("doit: %d\n", doit ());

  /* and unload it again */
  dlunload_TEST2 ();

  return 0;
}
