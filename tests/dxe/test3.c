/*
 * Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
 *
 * Usage of this library is not restricted in any way.  No warranty.
 *
 * This DXE module calls functions from another DXE module.
 * The functions in the external DXE module are resolved during
 * loading of this module because the other module is loaded with
 * the RTLD_GLOBAL mode flag set.
 *
 * Also note how the variable inside outer module is accessed.
 */


#include <stdio.h>

extern int test_func ();
extern int doit ();
extern int x_counter;

void test_reexp ()
{
  /* Call the test_func() module entry */
  printf ("test_func: %d\n", test_func ());

  /* Call the doit() module function */
  printf ("doit: %d\n", doit ());
  printf ("doit: %d\n", doit ());

  /* Set the x_counter variable inside the data segment of the outer module */
  x_counter = 100;
  printf ("doit: %d\n", doit ());
  printf ("doit: %d\n", doit ());
}
