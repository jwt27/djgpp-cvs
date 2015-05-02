/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

#include <libc/stubs.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libc/malloc.h>

#include "xmalloc.h"

struct mallinfo
mallinfo(void)
{
  struct mallinfo info;
  int b;
  BLOCK *block, **free_list, *slop;
  unsigned long free_size = 0;
#if NUMSMALL
  BLOCK **small_blocks;
#endif

  info.arena = __malloc_get_sbrked();

  info.ordblks = __malloc_get_chunks_in_use();
  /* To the chunks in use add the number of blocks on the free list
     and the slop, if non-NULL.  */
  free_list = __malloc_get_freelist();
  for (b = 0; b < 30; b++)
    for (block = free_list[b]; block; block = block->next)
    {
      info.ordblks++;
      free_size += block->size;
    }
  slop = __malloc_get_slop();
  if (slop)
  {
    info.ordblks++;
    free_size += slop->size;
  }

  info.smblks = info.hblks = info.hblkhd = info.usmblks = info.fsmblks = 0;
#if NUMSMALL
  small_blocks = __malloc_get_smallblocks();
  for (b = 0; b < NUMSMALL; b++)
    for (block = small_blocks[b]; block; block = block->next)
    {
      info.smblks++;
      info.fsmblks += block->size;
    }
#endif

  info.uordblks = __malloc_get_bytes_in_use();
  info.fordblks = free_size;
  info.keepcost = 0;	/* we don't support mallopt(M_KEEP, ...) */

  return info;
}

static int malloc_debug_level;

static BLOCK **recorded_blocks;
static int n_recorded_blocks;

static inline int
check_block(BLOCK *b)
{
  size_t size, endsz;
  size_t *endsz_ptr;
  extern unsigned __djgpp_selector_limit;

  if (!b)
  {
    if (malloc_debug_level)
      fprintf(stderr, "Corrupted heap block: addr=0x00000000\n");
    if (malloc_debug_level > 2)
      abort();
    return 0;
  }

  if ((((unsigned)b + 4) & 3) != 0)
  {
    if (malloc_debug_level)
      fprintf(stderr, "Heap block 0x%08x: address incorrectly aligned\n",
	      (unsigned)b);
    if (malloc_debug_level > 2)
      abort();
    return 0;
  }

  size = b->size;
  endsz_ptr = (size_t *)((char *)b + (b->size & ~1) + 4);
  if ((unsigned)endsz_ptr < 0x1000
      || (unsigned)endsz_ptr >= __djgpp_selector_limit)
  {
    if (malloc_debug_level)
    {
      fprintf(stderr,
	      "Bad size info in heap block: addr=0x%08x size=0x%08x\n",
	      (unsigned)b, (unsigned)endsz_ptr);
    }
    if (malloc_debug_level > 2)
      abort();
    return 0;
  }
  else
    endsz = *endsz_ptr;
  if (size != endsz)
  {
    if (malloc_debug_level)
    {
      fprintf(stderr,
	      "Corrupted heap block: addr=0x%08x size=0x%08x ENDSZ=0x%08x\n",
	      (unsigned)b, (unsigned)size, (unsigned)endsz);
    }
    if (malloc_debug_level > 2)
      abort();
    return 0;
  }
  return 1;
}

int
malloc_verify(void)
{
  BLOCK *b, **free_list;
#if NUMSMALL
  BLOCK **small_blks;
#endif
  int i, result = 1;
  extern unsigned __djgpp_selector_limit;
  BLOCK *slop = __malloc_get_slop();

  if (slop
      && ((unsigned)slop < 0x1000
	  || (unsigned)slop >= __djgpp_selector_limit
	  || (((unsigned)slop + 4) & 3) != 0))
  {
    result = 0;
    if (malloc_debug_level)
    {
      fprintf(stderr, "Bad slop 0x%08x\n", (unsigned)slop);
      if (malloc_debug_level > 2)
	abort();
    }
  }
  else if (slop)
    result &= check_block(slop);

  free_list = __malloc_get_freelist();
  if (free_list)
  {
    for (i = 0; i < 30; i++)
      for (b = free_list[i]; b; b = b->next)
      {
	if ((unsigned)b < 0x1000 || (unsigned)b >= __djgpp_selector_limit
	    || (((unsigned)b + 4) & 3) != 0)
	{
	  result = 0;
	  if (malloc_debug_level)
	  {
	    if ((((unsigned)b + 4) & 3) != 0)
	      fprintf(stderr,
		      "Free block 0x%08x: address incorrectly aligned\n",
		      (unsigned)b);
	    else
	      fprintf(stderr,
		      "Free block 0x%08x: address out of valid range\n",
		      (unsigned)b);
	    if (malloc_debug_level > 2)
	      abort();
	    else
	      break; /* we cannot use b->next, so go to next bucket */
	  }
	}
	else
	  result &= check_block(b);
      }
  }

#if NUMSMALL
  small_blks = __malloc_get_smallblocks();
  if (small_blks)
  {
    for (i = 0; i < NUMSMALL; i++)
      for (b = small_blks[i]; b; b = b->next)
      {
	if ((unsigned)b < 0x1000 || (unsigned)b >= __djgpp_selector_limit
	    || (((unsigned)b + 4) & 3) != 0)
	{
	  result = 0;
	  if (malloc_debug_level)
	  {
	    if ((((unsigned)b + 4) & 3) != 0)
	      fprintf(stderr,
		      "Free small block 0x%08x: address incorrectly aligned\n",
		      (unsigned)b);
	    else
	      fprintf(stderr,
		      "Free small block 0x%08x: address out of valid range\n",
		      (unsigned)b);
	    if (malloc_debug_level > 2)
	      abort();
	    else
	      break; /* we cannot use b->next, so go to next bucket */
	  }
	}
	else
	  result &= check_block(b);
      }
  }
#endif

  if (recorded_blocks)
    for (i = 0; i < n_recorded_blocks; i++)
      if (recorded_blocks[i])
	result &= check_block(recorded_blocks[i]);

  return result;
}

static int next_vacant;

static void
note_malloc(size_t size, void *ptr)
{
  int i;
  BLOCK *b = ptr;
  extern unsigned __djgpp_selector_limit;

  /* Record the allocated block.  */
  if (recorded_blocks)
  {
    /* See if we can find a vacant cell using the hint from the last
       invocation.  */
    if (!recorded_blocks[next_vacant])
    {
      recorded_blocks[next_vacant++] = b;
      if (next_vacant >= n_recorded_blocks)
	next_vacant = 0;
      i = 0;
    }
    else
    {
      for (i = 0; i < n_recorded_blocks; i++)
	if (!recorded_blocks[i])
	{
	  recorded_blocks[i] = b;
	  next_vacant = i + 1;
	  if (next_vacant >= n_recorded_blocks)
	    next_vacant = 0;
	  break;
	}
    }

    if (i == n_recorded_blocks && malloc_debug_level > 2)
      fprintf(stderr, "No space to record block 0x%08x of size 0x%08x\n",
	      (unsigned)b, (unsigned)size);
  }

  if (malloc_debug_level > 1)
  {
    if ((unsigned)b < 0x1000 || (unsigned)b >= __djgpp_selector_limit)
    {
      fprintf(stderr, "Block address 0x%08x out of range\n",
	      (unsigned)b);
      if (malloc_debug_level > 2)
	abort();
    }
    malloc_verify();
  }
}

static void
note_free(void *ptr)
{
  BLOCK *b = ptr;
  int i;

  /* Remove this block from the list of recorded blocks.  */
  if (recorded_blocks)
  {
    /* Check the cached vacant position, in case they are freeing
       memory in the same or reverse order as they allocated it.  */
    if (next_vacant < n_recorded_blocks - 1
	&& recorded_blocks[next_vacant + 1] == b)
      i = next_vacant + 1;
    else if (next_vacant > 0 && recorded_blocks[next_vacant - 1] == b)
      i = next_vacant - 1;
    else if (recorded_blocks[next_vacant] == b)
      i = next_vacant;
    else
      i = 0;
    for ( ; i < n_recorded_blocks; i++)
      if (recorded_blocks[i] == b)
      {
	recorded_blocks[i] = NULL;
	next_vacant = i;
	break;
      }

    if (i == n_recorded_blocks && malloc_debug_level > 1)
    {
      fprintf(stderr,
	      "Block 0x%08x freed, but is not known to be allocated\n",
	      (unsigned)ptr);
      if (malloc_debug_level > 2)
	abort();
    }
  }

  if (malloc_debug_level > 1)
    malloc_verify();
}

static void
note_malloc_fail(size_t size)
{
  fprintf(stderr, "Allocation failed for size 0x%08x\n", (unsigned)size);
}

static void
note_free_null(void)
{
  fprintf(stderr, "NULL pointer free'd\n");
}

static void
note_realloc(void *ptr, size_t size)
{
  if (malloc_debug_level > 1)
  {
    BLOCK *b = ptr;
    extern unsigned __djgpp_selector_limit;

    if ((unsigned)b < 0x1000 || (unsigned)b >= __djgpp_selector_limit)
    {
      fprintf(stderr, "Block address 0x%08x out of range\n", (unsigned)b);
      if (malloc_debug_level > 2)
	abort();
    }
    else if ((((unsigned)b + 4) & 3) != 0)
    {
      fprintf(stderr, "Block address 0x%08x incorrectly aligned\n",
	      (unsigned)b);
      if (malloc_debug_level > 2)
	abort();
    }
  }
}

int
malloc_debug(int level)
{
  int old_level = malloc_debug_level;
  static char debug_linebuf[160];	/* enough? */

  malloc_debug_level = level;

  if (malloc_debug_level)
  {
    /* Make sure we can use fprintf to stderr in the hooks without a
       risk of reentering malloc.  */
    if (stderr->_base == NULL && (stderr->_flag & _IONBF) == 0)
      if (setvbuf(stderr, debug_linebuf, _IOLBF, sizeof(debug_linebuf)))
	abort();

    __libc_malloc_hook = note_malloc;
    __libc_free_hook = note_free;
    __libc_realloc_hook = note_realloc;

    if (!recorded_blocks)
    {
      char *mdinfo = getenv("MALLOC_DEBUG"), *endp;
      if (mdinfo)
      {
	n_recorded_blocks = strtol(mdinfo, &endp, 0);
	if (n_recorded_blocks < 0 || n_recorded_blocks > 16*1024*1024)
	  n_recorded_blocks = 0;
      }
      if (!n_recorded_blocks)
	n_recorded_blocks = 100*1024;

      recorded_blocks = (BLOCK **) sbrk(n_recorded_blocks * sizeof(BLOCK *));
      if ((long)recorded_blocks == -1)
      {
	recorded_blocks = NULL;
	n_recorded_blocks = 0;
      }
      else
	memset(recorded_blocks, 0, n_recorded_blocks * sizeof(BLOCK *));
    }

    if (malloc_debug_level > 2)
    {
      __libc_malloc_fail_hook = note_malloc_fail;
      if (malloc_debug_level > 3)
	__libc_free_null_hook = note_free_null;
      else
	__libc_free_null_hook = NULL;
    }
    else
    {
      __libc_malloc_fail_hook = NULL;
      __libc_free_null_hook = NULL;
    }
  }
  else
  {
    __libc_malloc_hook = NULL;
    __libc_free_hook = NULL;
    __libc_realloc_hook = NULL;
    __libc_malloc_fail_hook = NULL;
    __libc_free_null_hook = NULL;
  }
  return old_level;
}

static inline void
print_block(BLOCK *b, const char *remark)
{
  if (b)
  {
    size_t size = b->size;
    size_t endsz = *(size_t *)((char *)b + (b->size & ~1) + 4);

    printf("  Addr=0x%08x   size=0x%08x   ENDSZ=0x%08x   %s\n",
	   (unsigned)b, (unsigned)size, (unsigned)endsz, remark);
  }
}

void
mallocmap(void)
{
  BLOCK *b, *slop, **free_list;
#if NUMSMALL
  BLOCK **small_blks;
#endif
  int i;

  slop = __malloc_get_slop();
  free_list = __malloc_get_freelist();

  if (slop)
    print_block(slop, "slop");

  if (free_list)
  {
    for (i = 0; i < 30; i++)
      for (b = free_list[i]; b; b = b->next)
	print_block(b, "free");
  }

#if NUMSMALL
  small_blks = __malloc_get_smallblocks();
  if (small_blks)
  {
    for (i = 0; i < NUMSMALL; i++)
      for (b = small_blks[i]; b; b = b->next)
	print_block(b, "small");
   }
#endif

  if (recorded_blocks)
  {
    for (i = 0; i < n_recorded_blocks; i++)
      print_block(recorded_blocks[i], "in use");
  }
}
