/*
 * File hcreate.c.
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

/*
 * It's impossible to have hash table with more than 0x20000000 elements
 * as sizeof(ENTRY) == 8.
 */
#define MAX_BUCKETS 0x20000000

static ENTRY *hash = NULL;
static size_t buckets = 0;
static size_t mask = -1;

int hcreate( size_t nel )
{
  size_t i;

  /* Fail anything > MAX_BUCKETS. */
  if( MAX_BUCKETS < nel )
  {
    return 0;
  }

  /* Increase with 1/3. */
  nel += nel/3;

  /* Decrease to MAX_BUCKETS if necessary. */
  if( MAX_BUCKETS < nel )
  {
    nel = MAX_BUCKETS;
  }

  /* Minimum is 128 elements. Algorithm doesn't depend on this, though. */
  i = 7;
  if( nel < 0x80 )
  {
    nel = 0x80;
  }

  /* Round up to next power-of-2. */
  while( (1U<<i) < nel )
  {
    i++;
  }
  
  buckets = 1U<<i;
  mask = buckets-1;

  hash = calloc(buckets, sizeof(ENTRY));

  return NULL != hash;
}

void hdestroy(void)
{
  free( hash );
  hash = NULL;
}

ENTRY *hsearch(ENTRY item, ACTION action)
{
  unsigned long hash_value, hash_value_start, increment;

  hash_value = _hash2v((const unsigned char *) item.key, &increment);
  hash_value &= mask;
  hash_value_start = hash_value;
  increment = 2*increment+1; /* Make sure increment is odd. */

  do {
    if( ! hash[hash_value].key )
    {
      if( FIND == action )
      {
	return NULL;
      }
      else
      {
	hash[hash_value] = item;
	return &hash[hash_value];
      }
    }
    else
    {
      if( ! strcmp(hash[hash_value].key, item.key) )
      {
	return &hash[hash_value];
      }
    
      hash_value += increment;
      hash_value &= mask;
    }
  } while( hash_value != hash_value_start );

  return NULL;
}
