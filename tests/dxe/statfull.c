/*
 * This is a full-blown example of the DXE3 capabilities.
 * It just requires a very special linking mode. Note the
 * absence of the export table; also note how both modules
 * (`test3p.dxe' and `test2x.dxe') are silently loaded.
 *
 *  Copyright (C) 2002 - Borca Daniel
 *  Email : dborca@yahoo.com
 *  Web   : http://www.geocities.com/dborca
 */


#include <stdio.h>

extern void test_reexp ();

void extern_func ()
{
  printf ("extern_func ()\n");
}

int main ()
{
  /* this resides in `test3p.dxe', which depends on `test2x.dxe' */
  test_reexp();

  return 0;
}
