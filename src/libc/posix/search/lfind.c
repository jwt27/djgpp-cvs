/*
 * File lfind.c.
 *
 * Copyright (C) 2005 Martin Str@"omberg <ams@ludd.ltu.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <search.h>
#include <stdlib.h>
#include <string.h>

/* Local helper function that does the real work. */
static void *
l_general(const void *key, void *base, size_t *nelp, size_t width, 
	  int(*compar)(const void *, const void *), int add)
{
  size_t i = 0;

  while( i < *nelp && (*compar)(base, key) )
  {
    base = ((unsigned char *)base) + width;
    i++;
  }

  if( i < *nelp )
  {
    return base;
  }

  if( add )
  {
    memcpy(base, key, width);
    (*nelp)++;
    return base;
  }

  return NULL;
}


void *
lfind(const void *key, void *base, size_t *nelp, size_t width, 
      int(*compar)(const void *, const void *))
{
  return l_general(key, base, nelp, width, compar, 0);
}


void *
lsearch(const void *key, void *base, size_t *nelp, size_t width, 
	int(*compar)(const void *, const void *))
{
  return l_general(key, base, nelp, width, compar, 1);
}

