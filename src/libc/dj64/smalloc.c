/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * smalloc - small memory allocator for dosemu.
 *
 * Author: Stas Sergeev
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "smalloc.h"

#define POOL_USED(p) (p->mn.used || p->mn.next)
#ifndef _min
#define _min(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
#define _PAGE_MASK (~(PAGE_SIZE-1))
#ifndef PAGE_ALIGN
#define PAGE_ALIGN(addr) (((addr)+PAGE_SIZE-1)&_PAGE_MASK)
#endif

static void smerror_dummy(int prio, const char *fmt, ...) FORMAT(printf, 2, 3);

static void (*smerr)(int prio, const char *fmt, ...)
	FORMAT(printf, 2, 3) = smerror_dummy;

static void smerror_dummy(int prio, const char *fmt, ...)
{
}

#define smerror(mp, ...) mp->smerr(3, __VA_ARGS__)

static int do_dump(struct mempool *mp, char *buf, int len)
{
    int pos = 0;
    struct memnode *mn;

#define DO_PRN(...) do { \
    if (pos >= len) \
      return -1; \
    pos += snprintf(buf + pos, len - pos, __VA_ARGS__); \
} while (0)
    DO_PRN("Total size: %zi\n", mp->size);
    DO_PRN("Available space: %zi (%zi used)\n", mp->avail,
            mp->size - mp->avail);
    DO_PRN("Largest free area: %zi\n", smget_largest_free_area(mp));
    DO_PRN("Memory pool dump:\n");
    for (mn = &mp->mn; mn; mn = mn->next)
        DO_PRN("\tarea: %zi bytes, %s\n",
                mn->size, mn->used ? "used" : "free");
    return 0;
}

void smdump(struct mempool *mp)
{
    char buf[16384];
    int err = do_dump(mp, buf, sizeof(buf));
    if (!err)
        mp->smerr(0, "%s", buf);
    else
        mp->smerr(3, "dump buffer overflow\n");
}

static FORMAT(printf, 3, 4)
void do_smerror(int prio, struct mempool *mp, const char *fmt, ...)
{
    char buf[16384];
    int err;
    size_t pos;
    va_list al;

    assert(prio != -1);
    va_start(al, fmt);
    pos = vsnprintf(buf, sizeof(buf), fmt, al);
    va_end(al);
    err = -1;
    if (pos < sizeof(buf))
        err = do_dump(mp, buf + pos, sizeof(buf) - pos);
    if (!err)
        mp->smerr(0, "%s", buf);
    else
        mp->smerr(3, "dump buffer overflow\n");
}

static int get_oom_pr(struct mempool *mp, size_t size)
{
    if (size <= smget_largest_free_area(mp))
	return -1;
    if (size > mp->size)
	return 2;
    if (size > mp->avail)
	return 1;
    return 0;
}

static void sm_uncommit(struct mempool *mp, void *addr, size_t size)
{
    /* align address up and align down size */
    uintptr_t a = (uintptr_t)addr;
    uintptr_t aa = PAGE_ALIGN(a);
    size_t aligned_size = (size - (aa - a)) & _PAGE_MASK;
    void *aligned_addr = (void *)aa;
    mp->avail += size;
    assert(mp->avail <= mp->size);
    if (!mp->uncommit)
      return;
    mp->uncommit(aligned_addr, aligned_size);
}

static int __sm_commit(struct mempool *mp, void *addr, size_t size,
	void *e_addr, size_t e_size)
{
  if (!mp->commit)
    return 1;
  if (!mp->commit(addr, size)) {
    smerror(mp, "SMALLOC: failed to commit %p %zi\n", addr, size);
    if (e_size)
      sm_uncommit(mp, e_addr, e_size);
    return 0;
  }
  return 1;
}

static int sm_commit(struct mempool *mp, void *addr, size_t size,
	void *e_addr, size_t e_size)
{
    /* align address down and align up size */
    uintptr_t a = (uintptr_t)addr;
    uintptr_t aa = a & _PAGE_MASK;
    size_t aligned_size = PAGE_ALIGN(size + (a - aa));
    void *aligned_addr = (void *)aa;
    int ok = __sm_commit(mp, aligned_addr, aligned_size, e_addr, e_size);
    if (ok) {
	assert(mp->avail >= size);
	mp->avail -= size;
    }
    return ok;
}

static int sm_commit_simple(struct mempool *mp, void *addr, size_t size)
{
  return sm_commit(mp, addr, size, NULL, 0);
}

static void mntruncate(struct memnode *pmn, size_t size)
{
  int delta = pmn->size - size;

  if (delta == 0)
    return;
  /* delta can be < 0 */
  if (pmn->next && !pmn->next->used) {
    struct memnode *nmn = pmn->next;

    assert(size > 0 && nmn->size + delta >= 0);

    nmn->size += delta;
    nmn->mem_area -= delta;
    pmn->size -= delta;
    if (nmn->size == 0) {
      pmn->next = nmn->next;
      free(nmn);
      assert(!pmn->next || pmn->next->used);
    }
  } else {
    struct memnode *new_mn;

    assert(size < pmn->size);

    new_mn = (struct memnode *)malloc(sizeof(struct memnode));
    new_mn->next = pmn->next;
    new_mn->size = delta;
    new_mn->used = 0;
    new_mn->mem_area = pmn->mem_area + size;

    pmn->next = new_mn;
    pmn->size = size;
  }
}

static struct memnode *find_mn(struct mempool *mp, unsigned char *ptr,
    struct memnode **prev)
{
  struct memnode *pmn, *mn;
  if (!POOL_USED(mp)) {
    smerror(mp, "SMALLOC: unused pool passed\n");
    return NULL;
  }
  for (pmn = NULL, mn = &mp->mn; mn; pmn = mn, mn = mn->next) {
    if (mn->mem_area > ptr)
      return NULL;
    if (mn->mem_area == ptr) {
      if (prev)
        *prev = pmn;
      return mn;
    }
  }
  return NULL;
}

static struct memnode *find_mn_at(struct mempool *mp, unsigned char *ptr)
{
  struct memnode *mn;
  for (mn = &mp->mn; mn; mn = mn->next) {
    if (mn->mem_area > ptr)
      return NULL;
    if (mn->mem_area + mn->size > ptr)
      return mn;
  }
  return NULL;
}

static struct memnode *smfind_free_area(struct mempool *mp, size_t size)
{
  struct memnode *mn;
  for (mn = &mp->mn; mn; mn = mn->next) {
    if (!mn->used && mn->size >= size)
      return mn;
  }
  return NULL;
}

static struct memnode *smfind_free_area_topdown(struct mempool *mp,
    unsigned char *top, size_t size)
{
  struct memnode *mn;
  struct memnode *mn1 = NULL;
  for (mn = &mp->mn; mn; mn = mn->next) {
    if (top && mn->mem_area + size > top)
      break;
    if (!mn->used && mn->size >= size)
      mn1 = mn;
  }
  return mn1;
}

static struct memnode *sm_alloc_fixed(struct mempool *mp, void *ptr,
    size_t size)
{
  struct memnode *mn;
  ptrdiff_t delta;
  if (!size || !ptr) {
    smerror(mp, "SMALLOC: zero-sized allocation attempted\n");
    return NULL;
  }
  if (!(mn = find_mn_at(mp, (unsigned char *)ptr))) {
    smerror(mp, "SMALLOC: invalid address %p on alloc_fixed\n", ptr);
    return NULL;
  }
  if (mn->used) {
    do_smerror(0, mp, "SMALLOC: address %p already allocated\n", ptr);
    return NULL;
  }
  delta = (uint8_t *)ptr - mn->mem_area;
  assert(delta >= 0);
  if (size + delta > mn->size) {
    int pr = get_oom_pr(mp, size);
    if (pr < 0)
      pr = 0;
    do_smerror(pr, mp, "SMALLOC: no space %zi at address %p\n", size, ptr);
    return NULL;
  }
  if (delta) {
    mntruncate(mn, delta);
    mn = mn->next;
    assert(!mn->used && mn->size >= size);
  }
  if (!sm_commit_simple(mp, mn->mem_area, size))
    return NULL;
  mn->used = 1;
  mntruncate(mn, size);
  assert(mn->size == size);
  memset(mn->mem_area, 0, size);
  return mn;
}

static struct memnode *sm_alloc_aligned(struct mempool *mp, size_t align,
    size_t size)
{
  struct memnode *mn;
  int delta;
  uintptr_t iptr;
  if (!size) {
    smerror(mp, "SMALLOC: zero-sized allocation attempted\n");
    return NULL;
  }
  /* power of 2 align */
  assert(__builtin_popcount(align) == 1);
  align--;
  if (!(mn = smfind_free_area(mp, size + align))) {
    do_smerror(get_oom_pr(mp, size), mp,
	    "SMALLOC: Out Of Memory on alloc, requested=%zu\n", size);
    return NULL;
  }
  /* insert small node to align the start */
  iptr = (uintptr_t)mn->mem_area;
  delta = ((iptr | align) - iptr + 1) & align;
  if (delta) {
    mntruncate(mn, delta);
    mn = mn->next;
    assert(!mn->used && mn->size >= size);
  }
  if (!sm_commit_simple(mp, mn->mem_area, size))
    return NULL;
  mn->used = 1;
  mntruncate(mn, size);
  assert(mn->size == size);
  memset(mn->mem_area, 0, size);
  return mn;
}

static struct memnode *sm_alloc_mn(struct mempool *mp, size_t size)
{
  return sm_alloc_aligned(mp, 1, size);
}

static struct memnode *sm_alloc_aligned_topdown(struct mempool *mp,
    unsigned char *top, size_t align, size_t size)
{
  struct memnode *mn;
  int delta;
  uintptr_t iptr;
  uintptr_t min_top;
  uintptr_t iend;
  if (!size) {
    smerror(mp, "SMALLOC: zero-sized allocation attempted\n");
    return NULL;
  }
  /* power of 2 align */
  assert(__builtin_popcount(align) == 1);
  align--;
  if (!(mn = smfind_free_area_topdown(mp, top, size + align))) {
    do_smerror(get_oom_pr(mp, size), mp,
	    "SMALLOC: Out Of Memory on alloc, requested=%zu\n", size);
    return NULL;
  }
  /* use top part of the found area */
  min_top = (uintptr_t)mn->mem_area + mn->size;
  if (top)
    min_top = _min(min_top, (uintptr_t)top);
  iptr = (min_top - size) & ~align;
  iend = iptr + size;
  delta = (uintptr_t)mn->mem_area + mn->size - iend;
  if (delta)
    mntruncate(mn, mn->size - delta);
  assert(iptr >= (uintptr_t)mn->mem_area);
  delta = iptr - (uintptr_t)mn->mem_area;
  if (delta) {
    mntruncate(mn, delta);
    mn = mn->next;
    assert(!mn->used && mn->size >= size);
  }
  if (!sm_commit_simple(mp, mn->mem_area, size))
    return NULL;
  mn->used = 1;
  mntruncate(mn, size);
  assert(mn->size == size);
  memset(mn->mem_area, 0, size);
  return mn;
}

static struct memnode *sm_alloc_topdown(struct mempool *mp, size_t size)
{
  return sm_alloc_aligned_topdown(mp, NULL, 1, size);
}

void *smalloc(struct mempool *mp, size_t size)
{
  struct memnode *mn = sm_alloc_mn(mp, size);
  if (!mn)
    return NULL;
  return mn->mem_area;
}

void *smalloc_fixed(struct mempool *mp, void *ptr, size_t size)
{
  struct memnode *mn = sm_alloc_fixed(mp, ptr, size);
  if (!mn)
    return NULL;
  assert(mn->mem_area == ptr);
  return mn->mem_area;
}

void *smalloc_aligned(struct mempool *mp, size_t align, size_t size)
{
  struct memnode *mn = sm_alloc_aligned(mp, align, size);
  if (!mn)
    return NULL;
  assert(((uintptr_t)mn->mem_area & (align - 1)) == 0);
  return mn->mem_area;
}

void *smalloc_topdown(struct mempool *mp, size_t size)
{
  struct memnode *mn = sm_alloc_topdown(mp, size);
  if (!mn)
    return NULL;
  return mn->mem_area;
}

void *smalloc_aligned_topdown(struct mempool *mp, unsigned char *top,
    size_t align, size_t size)
{
  struct memnode *mn = sm_alloc_aligned_topdown(mp, top, align, size);
  if (!mn)
    return NULL;
  assert(((uintptr_t)mn->mem_area & (align - 1)) == 0);
  return mn->mem_area;
}

int smfree(struct mempool *mp, void *ptr)
{
  struct memnode *mn, *pmn;
  if (!ptr)
    return -1;
  if (!(mn = find_mn(mp, (unsigned char *)ptr, &pmn))) {
    smerror(mp, "SMALLOC: bad pointer passed to smfree()\n");
    return -1;
  }
  if (!mn->used) {
    smerror(mp, "SMALLOC: attempt to free the not allocated region (double-free)\n");
    return -1;
  }
  assert(mn->size > 0);
  sm_uncommit(mp, mn->mem_area, mn->size);
  mn->used = 0;
  if (mn->next && !mn->next->used) {
    /* merge with next */
    assert(mn->next->mem_area >= mn->mem_area);
    mntruncate(mn, mn->size + mn->next->size);
  }
  if (pmn && !pmn->used) {
    /* merge with prev */
    assert(pmn->mem_area <= mn->mem_area);
    mntruncate(pmn, pmn->size + mn->size);
    mn = pmn;
  }
  return 0;
}

/* part of smrealloc() that covers the cases where the
 * extra memnode needs to be allocated for realloc */
static struct memnode *sm_realloc_alloc_mn(struct mempool *mp,
	struct memnode *pmn, struct memnode *mn,
	struct memnode *nmn, size_t size)
{
  struct memnode *new_mn;
  if (pmn && !pmn->used && pmn->size + mn->size +
	(nmn->used ? 0 : nmn->size) >= size) {
    /* move to prev memnode */
    size_t psize = _min(size, pmn->size);
    if (!sm_commit_simple(mp, pmn->mem_area, psize))
      return NULL;
    if (size > pmn->size + mn->size) {
      if (!sm_commit(mp, nmn->mem_area, size - pmn->size - mn->size,
	    pmn->mem_area, psize))
        return NULL;
    }
    pmn->used = 1;
    memmove(pmn->mem_area, mn->mem_area, mn->size);
    memset(pmn->mem_area + mn->size, 0, size - mn->size);
    mn->used = 0;
    if (size < pmn->size + mn->size) {
      size_t overl = size > pmn->size ? size - pmn->size : 0;
      sm_uncommit(mp, mn->mem_area + overl, mn->size - overl);
    }
    if (!nmn->used)	// merge with next
      mntruncate(mn, mn->size + nmn->size);
    mntruncate(pmn, size);
    new_mn = pmn;
  } else {
    /* relocate */
    new_mn = sm_alloc_mn(mp, size);
    if (!new_mn) {
      do_smerror(get_oom_pr(mp, size), mp,
	    "SMALLOC: Out Of Memory on realloc, requested=%zu\n", size);
      return NULL;
    }
    memcpy(new_mn->mem_area, mn->mem_area, mn->size);
    smfree(mp, mn->mem_area);
  }
  return new_mn;
}

void *smrealloc(struct mempool *mp, void *ptr, size_t size)
{
  struct memnode *mn, *pmn;
  if (!ptr)
    return smalloc(mp, size);
  if (!(mn = find_mn(mp, (unsigned char *)ptr, &pmn))) {
    smerror(mp, "SMALLOC: bad pointer passed to smrealloc()\n");
    return NULL;
  }
  if (!mn->used) {
    smerror(mp, "SMALLOC: attempt to realloc the not allocated region\n");
    return NULL;
  }
  if (size == 0) {
    smfree(mp, ptr);
    return NULL;
  }
  if (size == mn->size)
    return ptr;
  if (size < mn->size) {
    /* shrink */
    sm_uncommit(mp, mn->mem_area + size, mn->size - size);
    mntruncate(mn, size);
  } else {
    /* grow */
    struct memnode *nmn = mn->next;
    if (nmn && !nmn->used && mn->size + nmn->size >= size) {
      /* expand by shrinking next memnode */
      if (!sm_commit_simple(mp, nmn->mem_area, size - mn->size))
        return NULL;
      memset(nmn->mem_area, 0, size - mn->size);
      mntruncate(mn, size);
    } else {
      /* need to allocate new memnode */
      mn = sm_realloc_alloc_mn(mp, pmn, mn, nmn, size);
      if (!mn)
        return NULL;
    }
  }
  assert(mn->size == size);
  return mn->mem_area;
}

void *smrealloc_aligned(struct mempool *mp, void *ptr, int align, size_t size)
{
  struct memnode *mn, *pmn;
  assert(__builtin_popcount(align) == 1);
  if (!ptr)
    return smalloc_aligned(mp, align, size);
  if (!(mn = find_mn(mp, (unsigned char *)ptr, &pmn))) {
    smerror(mp, "SMALLOC: bad pointer passed to smrealloc()\n");
    return NULL;
  }
  if (!mn->used) {
    smerror(mp, "SMALLOC: attempt to realloc the not allocated region\n");
    return NULL;
  }
  if (size == 0) {
    smfree(mp, ptr);
    return NULL;
  }
  if (size == mn->size)
    return ptr;
  if ((uintptr_t)mn->mem_area & (align - 1)) {
    smerror(mp, "SMALLOC: unaligned pointer passed to smrealloc_aligned()\n");
    return NULL;
  }
  if (size < mn->size) {
    /* shrink */
    sm_uncommit(mp, mn->mem_area + size, mn->size - size);
    mntruncate(mn, size);
  } else {
    /* grow */
    struct memnode *nmn = mn->next;
    if (nmn && !nmn->used && mn->size + nmn->size >= size) {
      /* expand by shrinking next memnode */
      if (!sm_commit_simple(mp, nmn->mem_area, size - mn->size))
        return NULL;
      memset(nmn->mem_area, 0, size - mn->size);
      mntruncate(mn, size);
    } else {
      /* lazy impl */
      struct memnode *new_mn = sm_alloc_aligned(mp, align, size);
      if (!new_mn)
        return NULL;
      memcpy(new_mn->mem_area, mn->mem_area, mn->size);
      smfree(mp, mn->mem_area);
    }
  }
  assert(mn->size == size);
  return mn->mem_area;
}

int sminit(struct mempool *mp, void *start, size_t size)
{
  mp->size = size;
  mp->mn.size = size;
  mp->mn.used = 0;
  mp->mn.next = NULL;
  mp->mn.mem_area = (unsigned char *)start;
  mp->avail = size;
  mp->commit = NULL;
  mp->uncommit = NULL;
  mp->smerr = smerr;
  return 0;
}

static int do_sminit_com(struct mempool *mp, void *start, size_t size,
    int (*commit)(void *area, size_t size),
    int (*uncommit)(void *area, size_t size), int do_uncommit)
{
  sminit(mp, start, size);
  mp->commit = commit;
  mp->uncommit = uncommit;
  if (uncommit && do_uncommit)
    uncommit(start, size);
  return 0;
}

int sminit_com(struct mempool *mp, void *start, size_t size,
    int (*commit)(void *area, size_t size),
    int (*uncommit)(void *area, size_t size))
{
  return do_sminit_com(mp, start, size, commit, uncommit, 1);
}

int sminit_comu(struct mempool *mp, void *start, size_t size,
    int (*commit)(void *area, size_t size),
    int (*uncommit)(void *area, size_t size))
{
  return do_sminit_com(mp, start, size, commit, uncommit, 0);
}

void smfree_all(struct mempool *mp)
{
  struct memnode *mn;
  while (POOL_USED(mp)) {
    mn = &mp->mn;
    if (!mn->used)
      mn = mn->next;
    assert(mn && mn->used);
    smfree(mp, mn->mem_area);
  }
  assert(!mp->mn.next);
}

int smdestroy(struct mempool *mp)
{
  unsigned avail = mp->avail;

  smfree_all(mp);
  assert(mp->mn.size >= avail);
  /* return leaked size */
  return mp->mn.size - avail;
}

size_t smget_free_space(struct mempool *mp)
{
  return mp->avail;
}

size_t smget_free_space_upto(struct mempool *mp, unsigned char *top)
{
  struct memnode *mn;
  int cnt = 0;
  for (mn = &mp->mn; mn; mn = mn->next) {
    if (mn->mem_area + mn->size > top) {
      if (!mn->used && mn->mem_area < top)
        cnt += top - mn->mem_area;
      break;
    }
    if (!mn->used)
      cnt += mn->size;
  }
  return cnt;
}

size_t smget_largest_free_area(struct mempool *mp)
{
  struct memnode *mn;
  size_t size = 0;
  for (mn = &mp->mn; mn; mn = mn->next) {
    if (!mn->used && mn->size > size)
      size = mn->size;
  }
  return size;
}

int smget_area_size(struct mempool *mp, void *ptr)
{
  struct memnode *mn;
  if (!(mn = find_mn(mp, (unsigned char *)ptr, NULL))) {
    smerror(mp, "SMALLOC: bad pointer passed to smget_area_size()\n");
    return -1;
  }
  return mn->size;
}

void *smget_base_addr(struct mempool *mp)
{
  return mp->mn.mem_area;
}

void smregister_error_notifier(struct mempool *mp,
	void (*func)(int prio, const char *fmt, ...) FORMAT(printf, 2, 3))
{
  mp->smerr = func;
}

void smregister_default_error_notifier(
	void (*func)(int prio, const char *fmt, ...) FORMAT(printf, 2, 3))
{
  smerr = func;
}
