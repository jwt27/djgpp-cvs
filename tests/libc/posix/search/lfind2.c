/*
 * File lfind2.c.
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

#define N_ELEM       50
#define ELEM_SIZE    sizeof(char *)
#define COMPARE_FUN  my_p_strcmp

char *arr[N_ELEM];
size_t n_arr = 0;


int my_p_strcmp( const void *el1, const void *el2)
{
  return strcmp( *(const char * const *)el1, *(const char * const *)el2 );
}


int main(void)
{
  const char *nentry;
  char **entry;
  size_t i;

  /* Add entries. */
  nentry = "Alpha";
  entry = lsearch(&nentry, arr, &n_arr, ELEM_SIZE, COMPARE_FUN);
  if( !entry || n_arr != 1 )
  {
    printf("Error: failed to insert 'Alpha': entry = %p = '%s', n_arr = %ld.\n", 
	   entry, entry?*entry:"NULL pointer!", n_arr);
    exit(1);
  }
  nentry = "Bravo";
  entry = lsearch(&nentry, arr, &n_arr, ELEM_SIZE, COMPARE_FUN);
  if( !entry || n_arr != 2 )
  {
    printf("Error: failed to insert 'Bravo': entry = %p = '%s', n_arr = %ld.\n", 
	   entry, entry?*entry:"NULL pointer!", n_arr);
    exit(1);
  }
  nentry = "Charlie";
  entry = lsearch(&nentry, arr, &n_arr, ELEM_SIZE, COMPARE_FUN);
  if( !entry || n_arr != 3 )
  {
    printf("Error: failed to insert 'Charlie': entry = %p = '%s', n_arr = %ld.\n", 
	   entry, entry?*entry:"NULL pointer!", n_arr);
    exit(1);
  }

  /* Print out table. */
  printf("Array now contains:\n");
  for( i = 0; i < n_arr; i++ )
  {
    printf("\tIndex %ld: '%s'\n", i, arr[i]);
  }

  /* Search for the added entries. */
  nentry = "Bravo";
  entry = lfind(&nentry, arr, &n_arr, ELEM_SIZE, COMPARE_FUN);
  if( n_arr != 3 )
  {
    printf("Error: n_arr = %ld, expected 3.\n", n_arr);
    exit(1);
  }
  if( !entry || strcmp("Bravo", *entry) )
  {
    printf("Error: failed to find 'Bravo': entry = %p->'%s', n_arr = %ld.\n", 
	   entry, entry?*entry:"NULL pointer!", n_arr);
    exit(1);
  }
  nentry = "Charlie";
  entry = lfind(&nentry, arr, &n_arr, ELEM_SIZE, COMPARE_FUN);
  if( n_arr != 3 )
  {
    printf("Error: n_arr = %ld, expected 3.\n", n_arr);
    exit(1);
  }
  if( !entry || strcmp("Charlie", *entry) )
  {
    printf("Error: failed to find 'Charlie': entry = %p->'%s', n_arr = %ld.\n", 
	   entry, entry?*entry:"NULL pointer!", n_arr);
    exit(1);
  }
  nentry = "Alpha";
  entry = lfind(&nentry, arr, &n_arr, ELEM_SIZE, COMPARE_FUN);
  if( n_arr != 3 )
  {
    printf("Error: n_arr = %ld, expected 3.\n", n_arr);
    exit(1);
  }
  if( !entry || strcmp("Alpha", *entry) )
  {
    printf("Error: failed to find 'Alpha': entry = %p->'%s', n_arr = %ld.\n", 
	   entry, entry?*entry:"NULL pointer!", n_arr);
    exit(1);
  }

  /* Search for something not there. */
  nentry = "Zebra";
  entry = lfind(&nentry, arr, &n_arr, ELEM_SIZE, COMPARE_FUN);
  if( n_arr != 3 )
  {
    printf("Error: n_arr = %ld, expected 3.\n", n_arr);
    exit(1);
  }
  if( entry )
  {
    printf("Error: found 'Zebra': entry = %p->'%s', n_arr = %ld.\n", 
	   entry, entry?*entry:"NULL pointer!", n_arr);
    exit(1);
  }

  /* Try to add one already present. */
  nentry = "Bravo";
  entry = lsearch(&nentry, arr, &n_arr, ELEM_SIZE, COMPARE_FUN);
  if( !entry || n_arr != 3 )
  {
    printf("Error: failed to NOT insert 'Bravo': entry = %p, n_arr = %ld.\n", 
	   entry, n_arr);
    exit(1);
  }


  printf("All tests successful.\n");
  exit(0);
}
