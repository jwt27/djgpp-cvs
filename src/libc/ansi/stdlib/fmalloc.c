/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */

#include <stdlib.h>
#include <unistd.h>

char *buckets[32] = {0};
int bucket2size[32] = {0};

static inline int
size2bucket(int size)
{
  int rv = 0x1f;
  int bit = ~0x10;
  int i;

  if (size < 4) size = 4;
  size = (size+3)&~3;

  for (i=0; i<5; i++)
  {
    if (bucket2size[rv&bit] >= size)
      rv &= bit;
    bit>>=1;
  }
  return rv;
}

static void
init_buckets(void)
{
  unsigned b;
  for (b=0; b<32; b++)
    bucket2size[b] = (1<<b);
}

void *
malloc(size_t size)
{
  char *rv;
  int b;

  if (bucket2size[0] == 0)
    init_buckets();

  b = size2bucket(size);
  if (buckets[b])
  {
    rv = buckets[b];
    buckets[b] = *(char **)rv;
    return rv;
  }

  size = bucket2size[b]+4;
  rv = (char *)sbrk(size);

  *(int *)rv = b;
  rv += 4;
  return rv;
}

void
free(void *ptr)
{
  int b;
  if (ptr == 0)
    return;
  b = *(int *)((char *)ptr-4);
  *(char **)ptr = buckets[b];
  buckets[b] = ptr;
}

void *
realloc(void *ptr, size_t size)
{
  char *newptr;
  int oldsize;
  if (ptr == 0)
    return malloc(size);
  oldsize = bucket2size[*(int *)((char *)ptr-4)];
  if (size <= oldsize)
    return ptr;
  newptr = (char *)malloc(size);
  memcpy(newptr, ptr, oldsize);
  free(ptr);
  return newptr;
}
