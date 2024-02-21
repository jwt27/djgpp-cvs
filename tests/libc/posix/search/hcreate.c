/*
 * File hcreate.c.
 *
 * Copyright (C) 2005 Martin Str@"omberg <ams@ludd.ltu.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <errno.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BUF_LEN 2048
#define COMPARE_FUN  my_p_strcmp


char buffer[BUF_LEN];
char *program_name;
char **list;
size_t n_list = 0;
size_t size_list = 128;


static int my_p_strcmp( const void *el1, const void *el2)
{
  return strcmp( *(const char * const *)el1, *(const char * const *)el2 );
}


static void print_help_and_exit(int exit_value)
{
  fprintf(stderr, "Run like this: '%s <hash size> <input file>'\n", program_name);
  exit( exit_value );
}


/*
 * Makes sure that s is in list.
 * Returns:
 * 0 if s already was in list
 * 1 if s is new
 */
static int list_add(char **p)
{
  size_t old_n_list;

  if( size_list <= n_list+1 )
  {
    char **new_list;

    new_list = realloc(list, 2*size_list*sizeof(*list));
    if( ! new_list )
    {
      fprintf(stderr, "Failed to allocate memory (%ld bytes) for check list.\n",
	      2*size_list*sizeof(*list));
      print_help_and_exit(4);
    }

    list = new_list;
    size_list *= 2;
  }

  old_n_list = n_list;
  lsearch(p, list, &n_list, sizeof(char *), COMPARE_FUN);

  return old_n_list != n_list;
}


int main(int argc, char *argv[])
{
  char ch, *retp, scratch[32];
  ENTRY *e_p, item;
  FILE *in;
  int buckets, ret;
  size_t len, lines;

  program_name = argv[0];

  if( argc != 3 )
  {
    print_help_and_exit(1);
  }

  ret = sscanf(argv[1], "%i%c", &buckets, &ch);
  if( ret != 1 )
  {
    print_help_and_exit(2);
  }

  if( buckets <= 0 )
  {
    fprintf(stderr, "%d buckets makes no sense. Using 1000.\n", buckets);
    buckets = 1000;
  }

  in = fopen(argv[2], "r");
  if( ! in )
  {
    fprintf(stderr, "Failed to open input file '%s', errno = %d.\n",
	    argv[2], errno);
    print_help_and_exit(3);
  }

  list = malloc(size_list*sizeof(*list));
  if( ! list )
  {
    fprintf(stderr, "Failed to allocate memory for check list.\n");
    return 4;
  }

  if( ! hcreate(buckets) )
  {
    fprintf(stderr, "hcreate(%d) failed.\n", buckets);
    return 5;
  }

  item.data = NULL;

  /* Check for non-existing elements. */
  item.key = scratch;
  item.key[0] = 0;
  e_p = hsearch(item, FIND);
  if( e_p )
  {
    printf("ERROR: found element with key '%s' but hash is empty.\n",
	   item.key);
    return 42;
  }
  strcpy(item.key, "Baba Yaga");
  e_p = hsearch(item, FIND);
  if( e_p )
  {
    printf("ERROR: found element with key '%s' but hash is empty.\n",
           item.key);
    return 42;
  }

  /* Read file. */
  lines = 0;
  retp = fgets(buffer, BUF_LEN, in);
  while( retp )
  {
    lines++;

    /* Insert every line. */
    len = strlen(buffer);
    item.key = malloc((len+1)*sizeof(char));
    if( ! item.key )
    {
      fprintf(stderr, "Out of memory after inserting %lu elements from %lu lines.\n",
	      n_list, lines);
      return 4;
    }
    strcpy(item.key, buffer);
    ret = list_add(&item.key);
    e_p = hsearch(item, FIND);
    if( ret )
    {
      if( e_p )
      {
	printf("ERROR at the %luth element from %lu lines: new element\n"
	       "('%s') not in list, but found in hash.\n",
	       n_list, lines, item.key);
	return 42;
      }
      e_p = hsearch(item, ENTER);
      if( ! e_p )
      {
	printf("Hash full when inserting the %luth element from %lu lines (new element is\n"
	       "'%s').\n",
	       n_list, lines, item.key);
	return 0;
      }
    }
    else
    {
      if( ! e_p )
      {
	printf("ERROR at the %luth element from %lu lines: old element\n"
               "('%s') in list, but not found in hash.\n",
	       n_list, lines, item.key);
	return 42;
      }
    }

    retp = fgets(buffer, BUF_LEN, in);
  }

  /* Check for non-existing element. */

  printf("Test done. Inserted %lu elements from %lu lines.\n",
	 n_list, lines);

  return 0;
}
