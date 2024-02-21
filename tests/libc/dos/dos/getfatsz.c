/*
 * File getfatsz.c.
 *
 * Copyright (C) 2000 Martin Str@"omberg <ams@ludd.luth.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <stdio.h>
#include <dos.h>

int main(int argc, char *argv[])
{
  int ret;

  if( argc == 2
   && 'A' <= argv[1][0]
   && argv[1][0] <= 'Z' )
  {

    ret = _get_fat_size(argv[1][0] - 'A' + 1);
    fprintf(stderr, "_get_fat_size returned %d.\n", ret);

    return(0);
  }
  else
  {
    fprintf(stderr, "%s: run like this '%s C'.\n", argv[0], argv[0]);

    return(1);
  }

}
