/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */

#define brk __brk	/* Namespace fix; call crt0 code directly */
#include <libc/stubs.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libc/malloc.h>
#include "xmalloc.h"

static BLOCK *slop = 0;
static BLOCK *freelist[30];
#if NUMSMALL
static BLOCK *smallblocks[NUMSMALL];
#endif

static unsigned long malloc_bytes_in_use;
static unsigned long malloc_chunks_in_use;
static unsigned long malloc_sbrked;

#if DEBUG
static void
check(BLOCK *b)
{
  printf("check %08x %d %08x %d\n", b, b->size, &(ENDSZ(b)), ENDSZ(b));
}
#define CHECK(p) do { check(p); assert(p->size == ENDSZ(p)); consistency(); } while (0)
#define CHECK1(p) do { check(p); assert(p->size == ENDSZ(p)); } while (0)
static void
consistency()
{
#if 0
  int b;
  BLOCK *bl;
  if (slop)
    CHECK1(slop);
  for (b=0; b<32; b++)
    for (bl=freelist[b]; bl; bl=bl->next)
      CHECK1(bl);
#endif
}
#else
#define CHECK(p)
#endif

static inline BLOCK *
split_block(BLOCK *b, size_t size)
{
  BLOCK *rv = (BLOCK *)((char *)b + size+8);
#if DEBUG
  printf("  split %u/%08x to %u/%08x, %u/%08x\n",
	 b->size, b, size, b, b->size - size - 8, rv);
#endif
  rv->size = b->size - size - 8;
  rv->bucket = -1;
  b->size = size;
  ENDSZ(b) = b->size;
  ENDSZ(rv) = rv->size;
  CHECK(b);
  CHECK(rv);
  return rv;
}

/* These hooks can be used to gain access to the innards of memory
   allocation routines without bloating each program's size (since
   malloc is called from the startup code) or adverse effects on
   run-time performance of programs which don't call these hooks or
   malloc debug routines (which call them internally).  */
BLOCK **
__malloc_get_freelist(void)
{
  return freelist;
}

BLOCK *
__malloc_get_slop(void)
{
  return slop;
}

#if NUMSMALL
BLOCK **
__malloc_get_smallblocks(void)
{
  return smallblocks;
}
#endif

unsigned long
__malloc_get_bytes_in_use(void)
{
  return malloc_bytes_in_use;
}

unsigned long
__malloc_get_chunks_in_use(void)
{
  return malloc_chunks_in_use;
}

unsigned long
__malloc_get_sbrked(void)
{
  return malloc_sbrked;
}

void (*__libc_malloc_hook)(size_t, void *);
void (*__libc_malloc_fail_hook)(size_t);
void (*__libc_free_hook)(void *);
void (*__libc_free_null_hook)(void);
void (*__libc_realloc_hook)(void *, size_t);

#define RET(rv) \
  do { 				     \
    CHECK(rv);			     \
    ENDSZ(rv) |= 1;		     \
    malloc_bytes_in_use += rv->size; \
    malloc_chunks_in_use++;	     \
    rv->size |= 1;		     \
    return DATA(rv);		     \
  } while (0)

void *
malloc(size_t size)
{
  int b, chunk_size;
  BLOCK *rv, **prev;
  static BLOCK *expected_sbrk = 0;

  /* Refuse ridiculously large requests right away.  Anything beyond
     2GB will be treated by sbrk as a negative request, i.e. as a
     request to _decrease_ the heap size.  */
  if (size > 0x7fffffffU - 0x10000U) /* sbrk rounds up to 64KB */
  {
    if (__libc_malloc_fail_hook)
      __libc_malloc_fail_hook(size);
    return 0;
  }

  if (size<ALIGN) size = ALIGN;
  size = (size+(ALIGN-1))&~(ALIGN-1);
#if DEBUG
  printf("malloc(%u)\n", size);
#endif

#if NUMSMALL
  if (size < SMALL)
  {
    rv = smallblocks[size/ALIGN];
    if (rv)
    {
      smallblocks[size/ALIGN] = rv->next;
      if (__libc_malloc_hook)
	__libc_malloc_hook(size, rv);
      malloc_bytes_in_use += rv->size;
      malloc_chunks_in_use++;
      return DATA(rv);
    }
  }
#endif

  if (slop && slop->size >= size)
  {
    rv = slop;
#if DEBUG
    printf("  using slop %u/%08x\n", slop->size, slop);
#endif
    if (slop->size >= size+MIN_SAVE_EXTRA)
    {
      slop = split_block(slop, size);
#if DEBUG
      printf("  remaining slop %u/%08x\n", slop->size, slop);
#endif
    }
    else
      slop = 0;
    if (__libc_malloc_hook)
      __libc_malloc_hook(size, rv);
    RET(rv);
  }

  b = size2bucket(size);
  prev = &(freelist[b]);
  for (rv=freelist[b]; rv; prev=&(rv->next), rv=rv->next)
  {
    if (rv->size >= size && rv->size < size+size/4)
    {
      *prev = rv->next;
      if (__libc_malloc_hook)
	__libc_malloc_hook(size, rv);
      RET(rv);
    }
  }

  while (b < 30)
  {
    prev = &(freelist[b]);
#if DEBUG
    printf("  checking bucket %d\n", b);
#endif
    for (rv=freelist[b]; rv; prev=&(rv->next), rv=rv->next)
      if (rv->size >= size)
      {
#if DEBUG
	printf("    found size %d/%08x\n", rv->size, rv);
#endif
	*prev = rv->next;
	if (rv->size >= size+MIN_SAVE_EXTRA)
	{
#if DEBUG
	  printf("    enough to save\n");
#endif
	  if (slop)
	  {
	    b = b2bucket(slop);
#if DEBUG
	    printf("    putting old slop %u/%08x on free list %d\n",
		   slop->size, slop, b);
#endif
	    slop->next = freelist[b];
	    freelist[b] = slop;
	  }
	  slop = split_block(rv, size);
#if DEBUG
	  printf("    slop size %u/%08x\n", slop->size, slop);
#endif
	}
	if (__libc_malloc_hook)
	  __libc_malloc_hook(size, rv);
	RET(rv);
      }
    b++;
  }

  /* Make sure sbrk below returns an aligned address so data is aligned */
  brk((void *)( (int)((char *)sbrk(0)+(ALIGN-1)) & ~(ALIGN-1) ));
  
  chunk_size = size+16; /* two ends plus two placeholders */
  rv = (BLOCK *)sbrk(chunk_size);
  if (rv == (BLOCK *)(-1))
  {
    if (__libc_malloc_fail_hook)
      __libc_malloc_fail_hook(size);
    return 0;
  }
  malloc_sbrked += chunk_size;
#if DEBUG
  printf("sbrk(%d) -> %08x, expected %08x\n", chunk_size, rv, expected_sbrk);
#endif
  if (rv == expected_sbrk)
  {
    expected_sbrk = (BLOCK *)((char *)rv + chunk_size);
    /* absorb old end-block-marker */
#if DEBUG
    printf("  got expected sbrk\n");
#endif
    rv = (BLOCK *)((char *)rv - 4);
  }
  else
  {
    expected_sbrk = (BLOCK *)((char *)rv + chunk_size);
#if DEBUG
    printf("    disconnected sbrk\n");
#endif
    /* build start-block-marker */
    rv->size = 1;
    rv = (BLOCK *)((char *)rv + 4);
    chunk_size -= 8;
  }
  rv->size = chunk_size - 8;
  ENDSZ(rv) = rv->size;
  AFTER(rv)->size = 1;
  CHECK(rv);

  if (__libc_malloc_hook)
    __libc_malloc_hook(size, rv);
  RET(rv);
}

/* Remove block of memory from the free list.  */
static inline void
remove_freelist (BLOCK *b)
{
  int bu;
  BLOCK *bp, **bpp;

  bu = b2bucket(b);
#if DEBUG
  printf("bucket for %u/%08x is %d\n", b->size, b, bu);
#endif
  bpp = freelist + bu;
  for (bp = freelist[bu]; bp; bpp=&(bp->next), bp=bp->next)
  {
#if DEBUG
    printf("  %08x", bp);
#endif
    if (bp == b)
    {
#if DEBUG
      printf("\n  snipping %u/%08x from freelist[%d]\n", bp->size, bp, bu);
#endif
      *bpp = bp->next;
      break;
    }
  }
}

/* Insert block of memory into free list.  */
static inline void
insert_freelist(BLOCK *block)
{
  int bu = b2bucket(block);
  block->next = freelist[bu];
  freelist[bu] = block;
}

static inline BLOCK *
merge(BLOCK *a, BLOCK *b, BLOCK *c)
{
#if DEBUG
  printf("  merge %u/%08x + %u/%08x = %u\n",
	 a->size, a, b->size, b, a->size+b->size+8);
#endif

  CHECK(a);
  CHECK(b);
  CHECK(c);
  if (c == slop)
  {
#if DEBUG
    printf("  snipping slop %u/%08x\n", slop->size, slop);
#endif
    slop = 0;
  }
  else
    remove_freelist(c);
  CHECK(c);

  a->size += b->size + 8;
  a->bucket = -1;
  ENDSZ(a) = a->size;

  CHECK(a);
  return a;
}

void
free(void *ptr)
{
  BLOCK *block;
  if (ptr == 0)
  {
    if (__libc_free_null_hook)
      __libc_free_null_hook();
    return;
  }
  block = (BLOCK *)((char *)ptr-4);
  if (__libc_free_hook)
    __libc_free_hook(block);

#if NUMSMALL
  if (block->size < SMALL)
  {
    block->next = smallblocks[block->size/ALIGN];
    smallblocks[block->size/ALIGN] = block;
    malloc_bytes_in_use -= block->size;
    malloc_chunks_in_use--;
    return;
  }
#endif

  block->size &= ~1;
  malloc_bytes_in_use -= block->size;
  malloc_chunks_in_use--;
  ENDSZ(block) &= ~1;
  block->bucket = -1;
#if DEBUG
  printf("free(%u/%08x)\n", block->size, block);
#endif

  CHECK(block);
  if (! (AFTER(block)->size & 1))
  {
    CHECK(AFTER(block));
  }
  if (! (BEFSZ(block) & 1))
  {
    CHECK(BEFORE(block));
    block = merge(BEFORE(block), block, BEFORE(block));
  }
  CHECK(block);
  if (! (AFTER(block)->size & 1))
  {
    CHECK(AFTER(block));
    block = merge(block, AFTER(block), AFTER(block));
  }
  CHECK(block);

  insert_freelist(block);
  CHECK(block);
}

/* Try to increase the size of the allocated block. */
static BLOCK *
realloc_inplace(BLOCK *cur, size_t old_size, size_t new_size)
{
  BLOCK *after;
  size_t after_sz, alloc_delta;
  int extra_space;
  char is_slop_ptr;
  
  after = AFTER(cur);
  after_sz = after->size;
  new_size = (new_size + (ALIGN-1)) & ~(ALIGN-1);

  /* Fail if the block following the one being extended is in use.  */
  if (after_sz & 1)
    return NULL;

  /* Size of block increase.  */
  alloc_delta = new_size - old_size;

  /* Fail if the free block is not large enough.  */
  if (alloc_delta > after_sz)
    return NULL;

  extra_space = (after_sz - alloc_delta);
  is_slop_ptr = (after == slop);

  if (!is_slop_ptr)
  {
    /* Remove the block from free list.  */
    after->bucket = -1;
    remove_freelist(after);
  }

  /* Expand the block by shrinking or mergin
     with the free block that follows it.  */
  if (extra_space > 8)
  {
    /* Take part of the free block (or slop) and
       give to the block being expanded.  */
    BLOCK *after2 = (BLOCK *)((char *)after + alloc_delta);
    after2->size = after_sz - alloc_delta;
    after2->bucket = -1;
    ENDSZ(after2) = after2->size;
    cur->size += alloc_delta;
    ENDSZ(cur) = cur->size;
    malloc_bytes_in_use += alloc_delta;
    if (is_slop_ptr)
      slop = after2;
    else
      insert_freelist(after2);
  }
  else
  {
    /* Merge the entire free block with the block being expanded.  */
    cur->size += after_sz + 8;
    ENDSZ(cur) = cur->size;
    malloc_bytes_in_use += after_sz + 8;
    if (is_slop_ptr)
      slop = 0;
  }
  return cur;
}

void *
realloc(void *ptr, size_t size)
{
  BLOCK *b;
  char *newptr;
  size_t copysize;

  if (ptr == 0)
    return malloc(size);

  b = (BLOCK *)((char *)ptr-4);
  if (__libc_realloc_hook)
    __libc_realloc_hook(b, size);
  copysize = b->size & ~1;
  if (size <= copysize)
  {
#if 0
    if (copysize < 2*MIN_SAVE_EXTRA
	|| (size >= copysize-512 && size >= copysize/2))
#endif
      return ptr;
    copysize = size;
  }

  /* Try to increase the size of the allocation by extending the block.  */
  if (realloc_inplace(b, copysize, size))
    return ptr;

  newptr = (char *)malloc(size);
#if DEBUG
  printf("realloc %u %u/%08x %08x->%08, %u\n",
	 size, b->size & ~1, b, ptr, newptr, copysize);
#endif
  if (!newptr)
    return NULL;
  memcpy(newptr, ptr, copysize);
  free(ptr);
  return newptr;
}
