/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */
#include "code32.h"

#define RESET_MAP(no) map[(no) >> 3] &= ~(1 << ((no) & 7))
#define SET_MAP(no) map[(no) >> 3] |= (1 << ((no) & 7))
#define MAP_SET_P(no) (map[(no) >> 3] & (1 << ((no) & 7)))

#define LINEAR_RESET_MAP(no) linear_map[(no) >> 3] &= ~(1 << ((no) & 7))
#define LINEAR_SET_MAP(no) linear_map[(no) >> 3] |= (1 << ((no) & 7))
#define LINEAR_MAP_SET_P(no) (linear_map[(no) >> 3] & (1 << ((no) & 7)))
/* ---------------------------------------------------------------------- */
typedef struct free_item
{
  struct free_item *next;
  int size;
} free_item;

free_item *free_list;

/* Alignment of malloc'ed memory.  Must be at least as big as
   `sizeof (struct free_item)', pt. 0x8 bytes.  */
#define MALLOC_ALIGNMENT 0x10
/* ---------------------------------------------------------------------- */
/* Bitmap for allocation of physical memory.  */
static word8 *map; /* [PAGE_SIZE] */

/* Pointer to location after server memory.  ("break"-like beast.)  */
static void *core_end;

static word32 *page_dir;

static int page_min, page_max, page_search, page_free_count;

/* Bitmap for allocation of linear memory.  */
static word8 *linear_map; /* [PAGE_SIZE] */
/* ---------------------------------------------------------------------- */
/* Clear translation lookaside buffer.  */
static inline void
clear_tlb (void)
{
  __cpu_set_cr3 (__cpu_get_cr3 ());
}
/* ---------------------------------------------------------------------- */
void
map_page_table (word32 physical, word32 linear)
{
  page_dir[linear >> (2 * PAGE_SIZE_LOG - 2)] = physical;
  clear_tlb ();
}
/* ---------------------------------------------------------------------- */
static inline void *
map_page (word32 physical, word32 *table, word32 linear)
{
  table[(linear / PAGE_SIZE) % (PAGE_SIZE / 4)] = physical;
  return LINEAR_TO_PTR (linear);
}
/* ---------------------------------------------------------------------- */
/* Initialize the memory allocation system.  */

void
malloc_init (void)
{
  page_dir = LINEAR_TO_PTR (page_dir_linear);
  core_end =
    (void *)(((word32)&code32_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
  free_list = 0;

  if (memory_source == MEMORY_FROM_VCPI)
    {
      /* Use page that would hold dosmem page table in non-VCPI case.  */
      map = (void *)page_dir + 2 * PAGE_SIZE;
      page_free_count =
	memory_ext_last - ((server_ext_size + PAGE_SIZE - 1) / PAGE_SIZE);
    }
  else
    {
      page_min = page_search =
	memory_ext_first + ((server_ext_size + PAGE_SIZE - 1) / PAGE_SIZE) + 1;
      page_max = memory_ext_last;
      map = map_page ((page_search - 1) * PAGE_SIZE + (PT_P | PT_W | PT_U),
		      LINEAR_TO_PTR (server_page_table_linear),
		      PTR_TO_LINEAR (core_end));
      clear_tlb ();
      core_end += PAGE_SIZE;

      page_free_count = page_max - page_min + 1;
    }
  memset (map, 0, PAGE_SIZE);

  linear_map = xmalloc (PAGE_SIZE);
  memset (linear_map, 0, PAGE_SIZE);
  LINEAR_SET_MAP (0);  /* dosmem + vcpi server controlled.  */
  LINEAR_SET_MAP (1);  /* Server.  */
}
/* ---------------------------------------------------------------------- */
/* Prepare for shutdown by deallocating all VCPI pages.  */

void
malloc_shutdown (void)
{
  if (memory_source == MEMORY_FROM_VCPI)
    {
      int i;

      interrupts_safe = 0;
      for (i = memory_ext_first; i <= memory_ext_last; i++)
	if (MAP_SET_P (i))
	  pfree (i);
    }
}
/* ---------------------------------------------------------------------- */
/* Allocate a page of memory.  Returns the page number.  */

int
palloc (void)
{
  int page_no;

  if (memory_source == MEMORY_FROM_VCPI)
    {
      word32 physical;
      int error;

      if (interrupts_safe)
	{
	  asm volatile
	    ("
		movw	$0xde04,%%ax
		lcall	_vcpi_entry
		movzbl	%%ah,%%eax"
	     : "=d" (physical), "=a" (error));
	}
      else
	{
	  __dpmi_regs regs;

	  regs.x.ax = VCPI_ALLOC_PAGE;
	  server_int (INT_VCPI, &regs);
	  error = (regs.h.ah != 0);
	  physical = regs.d.edx;
	}

      if (error) return -1;
      page_no = physical >> PAGE_SIZE_LOG;
    }
  else
    {
      /* Simple circular search in the page space.  */
      page_no = page_search;

      while (MAP_SET_P (page_no))
	{
	  page_no = (page_no == page_max) ? page_min : (page_no + 1);
	  if (page_no == page_search) return -1;
	}
      page_search = page_no;
    }

  SET_MAP (page_no);
  page_free_count--;
  return page_no;
}
/* ---------------------------------------------------------------------- */
/* Free a give page.  */

void
pfree (int page_no)
{
  RESET_MAP (page_no);
  if (memory_source == MEMORY_FROM_VCPI)
    {
      if (interrupts_safe)
	asm volatile
	  ("
		movw	$0xde05,%%ax
		lcall	_vcpi_entry"
	   : /* No output */
	   : "d" (page_no << PAGE_SIZE_LOG)
	   : "eax", "edx");
      else
	{
	  __dpmi_regs regs;
	  regs.x.ax = VCPI_FREE_PAGE;
	  regs.d.edx = page_no << PAGE_SIZE_LOG;
	  server_int (INT_VCPI, &regs);
	}
    }
  page_free_count++;
}
/* ---------------------------------------------------------------------- */
/* Allocate a number of extra pages for the malloc system, returning the
   address of the mapped memory.  */

static void *
morecore (int pages)
{
  void *result = core_end;
  int count = 0;

  while (pages > 0)
    {
      int page = palloc ();
      if (page == -1)
	{
	  /* Not enough memory.  Relaese partial allocation.  */
	  while (count--)
	    {
	      core_end -= PAGE_SIZE;
	      pfree (ptr_to_physical (core_end) / PAGE_SIZE);
	    }
	  return 0;
	}
      map_page (page * PAGE_SIZE + (PT_P | PT_W | PT_U),
		LINEAR_TO_PTR (server_page_table_linear),
		PTR_TO_LINEAR (core_end));
      core_end += PAGE_SIZE;
      pages--, count++;
    }
  clear_tlb ();
  return result;
}
/* ---------------------------------------------------------------------- */
/* This version of free accepts NULL pointers and does nothing.  */

void
free (void *p)
{
  if (p)
    {
      free_item *f = p - 0x10;
      f->next = free_list;
      free_list = f;
    }
}
/* ---------------------------------------------------------------------- */
/* Simple and naive -- good enough.  MALLOC_ALIGNMENT is guaranteed.  */

void *
malloc (int size)
{
  free_item *f, *fb, *f_best, *fb_best;
  int best_size = 0x7fffffff;

  size = (size + 2 * MALLOC_ALIGNMENT - 1) & ~(MALLOC_ALIGNMENT - 1);
  for (f = free_list, f_best = fb = fb_best = 0;
       f && best_size != size;
       fb = f, f = f->next)
    if (f->size >= size && f->size < best_size)
      f_best = f, fb_best = fb, best_size = f->size;

  if (!f_best)
    {
      /* No item is large enough.  Allocate some new memory.  */
      best_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
      f_best = morecore (best_size >> PAGE_SIZE_LOG);
      if (!f_best) return 0;
      f_best->next = free_list;
      f_best->size = best_size;
      fb_best = 0;
      free_list = f_best;
    }

  if (size == best_size)
    {
      /* We have a complete match.  Good, unlink it from the free list.  */
      if (fb_best)
	fb_best->next = f_best->next;
      else
	free_list = free_list->next;
    }
  else
    {
      /* The old block is larger so split it.  */
      if (fb_best)
	f = ((void *)(fb_best->next) += size);
      else
	f = ((void *)free_list += size);
      f->next = f_best->next;
      f->size = best_size - size;
      f_best->size = size;
    }
  return (void *)f_best + MALLOC_ALIGNMENT;
}
/* ---------------------------------------------------------------------- */
void *
xmalloc (int size)
{
  void *res = malloc (size);
  if (res == 0)
    {
      MESSAGE ("Out of extended memory.\r\n");
      shutdown (1);
    }
  return res;
}
/* ---------------------------------------------------------------------- */
/* For server memory only.  */

word32
ptr_to_physical (void *p)
{
  word32 *table = LINEAR_TO_PTR (server_page_table_linear);
  word32 linear = PTR_TO_LINEAR (p);

  return
    (table[(linear / PAGE_SIZE) % (PAGE_SIZE / 4)] & ~(PAGE_SIZE - 1)) |
      (linear & (PAGE_SIZE - 1));
}
/* ---------------------------------------------------------------------- */
/* Allocation of linear memory.  Granularity is 4M.  */

word32
linear_alloc (word32 size)
{
  int gran_log = 2 * PAGE_SIZE_LOG - 2;
  int gran = 1 << gran_log;
  int blocks = 1 << (32 - gran_log);
  int count = (size + (gran - 1)) / gran;
  int first;

  /* First two always used.  */
  for (first = 2; first + count <= blocks; )
    {
      int test = first + count - 1;

      while (!LINEAR_MAP_SET_P (test))
	{
	  if (test > first)
	    test--;
	  else
	    {
	      for (test = first; test < first + count; test++)
		LINEAR_SET_MAP (test);
	      return (word32)first << gran_log;
	    }
	}
      first = test + 1;
    }
  return 0;
}
/* ---------------------------------------------------------------------- */
void
linear_free (word32 linear, word32 size)
{
  int gran_log = 2 * PAGE_SIZE_LOG - 2;
  int gran = 1 << gran_log;
  int count = (size + (gran - 1)) / gran;
  int i = linear >> gran_log;

  while (count-- > 0)
    {
      LINEAR_RESET_MAP (i);
      i++;
    }
}
/* ---------------------------------------------------------------------- */
/* Allocate virtual memory for a page.  FIXME: currently just allocates
   physical memory.  */

int
commit_page (word32 *page_table_entry)
{
  if ((*page_table_entry & PT_P) == 0)
    {
      int page = palloc ();
      if (page == -1) return 1;
      *page_table_entry = (page << PAGE_SIZE_LOG) | (PT_P | PT_W | PT_U);
    }
  return 0;
}
/* ---------------------------------------------------------------------- */
/* Deallocate virtual memory for a page.  FIXME: currently just deallocates
   physical memory.  */

int
uncommit_page (word32 *page_table_entry)
{
  if ((*page_table_entry & PT_P))
    {
      pfree (*page_table_entry >> PAGE_SIZE_LOG);
      *page_table_entry &= ~PT_P;
    }
  return 0;
}
/* ---------------------------------------------------------------------- */
void
physical_poke (word32 physical, word32 val)
{
  word32 offset = physical % PAGE_SIZE;
  word32 *p = LINEAR_TO_PTR (0x007ff000 + offset);
  word32 *entry = LINEAR_TO_PTR (server_page_table_linear) + (PAGE_SIZE - 4);

  /* Non present page tables are not cached so we don't need to clear the
     look-aside buffer to get the page in.  */
  *entry = (physical - offset) | (PT_U | PT_P | PT_W);
  *p = val;
  *entry = 0;
  clear_tlb ();
}
/* ---------------------------------------------------------------------- */
void
memory_info (int *free_pages)
{
  *free_pages = page_free_count;
}
/* ---------------------------------------------------------------------- */
