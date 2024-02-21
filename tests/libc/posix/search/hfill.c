/*
 * File hfill.c.
 * Test program that fills up a hash completely.
 *
 * Copyright (C) 2005 Martin Str@"omberg <ams@ludd.ltu.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[])
{
  char ch;
  ENTRY *e_p, item;
  int size = 1;
  size_t l;

  if( 2 < argc
    || ( argc == 2
      && ( ( argv[1][0] == '-' && argv[1][1] == 'h' )
       || 1 != sscanf(argv[1], "%i%c", &size, &ch) ) ) )
  {
    fprintf(stderr, "Run like this: '%s [hash size]' (hash size defaults to 1).\n",
	    argv[0]);
    return 1;
  }

  if( ! hcreate(size) )
  {
    fprintf(stderr, "hcreate(%d) failed.\n", size);
    return 1;
  }

  l = 2;
  item.data = NULL;
  item.key = malloc(l*sizeof(char));
  if( ! item.key )
  {
    fprintf(stderr, "Out of memory.\n");
    return 0;
  }
  item.key[0] = 'a';
  item.key[1] = 0;
  e_p = hsearch(item, ENTER);
  while( e_p )
  {
    l++;
    item.key = malloc(l*sizeof(char));
    if( ! item.key )
    {
      fprintf(stderr, "Out of memory.\n");
      return 0;
    }
    memset(item.key, 'a', l-1);
    item.key[ l-1 ] = 0;

    e_p = hsearch(item, ENTER);
  }

  printf("Managed to insert %lu elements.\n", l-2);

  return 0;

}
