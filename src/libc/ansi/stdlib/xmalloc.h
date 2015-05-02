/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

#define MIN_SAVE_EXTRA	64
#define BIG_BLOCK	4096

#define DEBUG 0

static inline int
size2bucket(size_t size)
{
  int rv=0;
  size>>=2;
  while (size)
  {
    rv++;
    size>>=1;
  }
  return rv;
}

static inline int
b2bucket(BLOCK *b)
{
  if (b->bucket == -1)
    b->bucket = size2bucket(b->size);
  return b->bucket;
}

BLOCK ** __malloc_get_freelist(void);
BLOCK * __malloc_get_slop(void);
BLOCK ** __malloc_get_smallblocks(void);
unsigned long __malloc_get_bytes_in_use(void);
unsigned long __malloc_get_chunks_in_use(void);
unsigned long __malloc_get_sbrked(void);
