/* --------- nmalloc.c ----------- */

/* Copyright (c) 2003, 2007 by Charles B. Falconer
   Licensed under the terms of the GNU LIBRARY GENERAL PUBLIC
   LICENSE and/or the terms of COPYING.DJ, all available at
   <http://www.delorie.com>.

   Bug reports to <mailto:djgpp@delorie.com>

   Revised 2007-01-04 to include calloc.
   Revised 2007-01-28 per bug report by Florian Xaver. *f1*
   Revised 2014, 2015 for use in DJGPP libc by Andris Pavenis <andris.pavenis@iki.fi>
*/

/* A re-implementation of malloc and friends for DJGPP 2.03/2.04
   This includes many bits modeled after DJs original scheme.
   This is NOT portable - it builds in knowledge of int size etc.
   i.e. unsigned ints and pointers are both 32 bits (size 4)

   The system is NOT thread and interrupt safe, although use of a
   suitable critical section call could make it such.  Nothing
   herein executes for any unusual length of time (with NDEBUG).
*/

/* Some critical tuning constants. Search for them:
   MINSBRK    controls minimal access to sbrk
   ALIGN      controls alignment of pointers
   SAVEMEMORY reduces overhead at expense of checkability
   INT_MAX    (system) controls maximum allocation quantum.
*/

/* To avoid unexpected problems, the default has been changed
   so we now require NEWMALLDBG to enable the original action
*/

#define MEMALIGN

#ifndef NEWMALLDBG
#  undef  NDEBUG
#  define NDEBUG
#else
#  undef  NDEBUG
#endif

#ifndef NDEBUG
   /* diddle these to control areas debugged */
#  define DEBUGM 1     /* malloc */
#  define DEBUGF 1     /* free */
#  define DEBUGR 1     /* realloc */
#else
#  define DEBUGM 0
#  define DEBUGF 0
#  define DEBUGR 0
   /* the HOOKABLE variant is only for development   */
   /* It allows some other package to define malloc, */
   /* free, realloc, and to call this package.       */
#  ifndef HOOKABLE
#     define nmalloc  malloc
#     define nfree    free
#     define nrealloc realloc
#     define nmemalign memalign
#     define ncalloc  calloc
#  else
#     define nmalloc  _malloc
#     define nfree    _free
#     define nrealloc _realloc
#     define nmemalign _memalign
#     define ncalloc   calloc  /* can't hook this */
#  endif
#  define fakesbrk sbrk
#endif

#define SAVEMEMORY 1  /* 0/1 to use/eliminate extra storage */

typedef unsigned char byte;
typedef unsigned int  ulong;

#include <stddef.h> /* offsetof() */
#include <stdlib.h> /* malloc, free, realloc, exit, EXIT_FAILURE */
#include <unistd.h> /* sbrk, write */
#include <signal.h> /* raise, SIGABRT */
#include <string.h> /* strlen, memmove, memcpy, memset */
#include <limits.h> /* CHAR_BIT, INT_MAX */
#include <malloc.h>
#include <libc/sysquery.h> /* available debugger linkages */

/* system dependant magic.  Only STDIN, STDERR used */
enum {STDIN = 0, STDOUT, STDERR, STDAUX, STDPRN}; /* handles */

/* Incorporation into the system should require deleting the
   following <nmalloc.h>, changing all references to nmalloc
   to malloc, nfree to free, nrealloc to realloc.  Change the
   single call to fakesbrk to sbrk.  Also normally set all
   DEBUGx values above to 0 in place of 1.  Later many
   routines will be made inline.  For debugging compilations
   are done with "/Dinline= " switch.  For production use
   the "/DNDEBUG=1" switch, which does all the above except
   the inlining. But see NEWMALLDBG above.
*/
#ifndef NDEBUG
#  include "nmalloc.h" /* while testing before name changes */
#else
#  ifdef HOOKABLE
#     include "hookmem.h"
#  endif
#endif

/* ============================================================
   Macros and storage for debugging, works before init on DJGPP
   WARNING - GNU extensions used here!!
   Note: many messages are designed for easy search with grep
         and also serve as comments.
*/
#if DEBUGM || DEBUGF || DEBUGR
#  include <stdio.h>     /* sprintf, for DEBUG only */
#  include "fakesbrk.h"  /* repeatable sbrk */
#  define EOL "\n"       /* for DEBUG printouts only, allow crlf */
   static char  dbgbuff[1024];
   static char *dbgbp = dbgbuff;
#  define DBGFLUSH do {if (dbgbp != dbgbuff) { \
                         /* write it out */ \
                         write(STDOUT, dbgbuff, strlen(dbgbuff)); \
                         dbgbp = dbgbuff; \
                      } \
                   } while (0)
#  define DBGEOLN do { \
                     DBGPRT(EOL); \
                     DBGFLUSH; \
                  } while (0)
#  define DBGPRT(msg, args...) do { \
                    if ((dbgbp - dbgbuff) > 924) DBGFLUSH; \
                    dbgbp +=sprintf(dbgbp, msg , ## args); \
                 } while (0)
#  define SHOWBLK(m, id) showblock(m, id)
#  if DEBUGM
#     define DBGPRTM(msg, args...) \
                     dbgbp +=sprintf(dbgbp, msg , ## args)
#     define SHOWBLKM(m, id) showblock(m, id)
#  else
#     define DBGPRTM(msg, args...)
#     define SHOWBLKM(m, id)
#  endif
#  if DEBUGF
#     define DBGPRTF(msg, args...) \
                     dbgbp +=sprintf(dbgbp, msg , ## args)
#     define SHOWBLKF(m, id) showblock(m, id)
#  else
#     define DBGPRTF(msg, args...)
#     define SHOWBLKF(m, id)
#  endif
#  if DEBUGR
#     define DBGPRTR(msg, args...) \
                     dbgbp +=sprintf(dbgbp, msg , ## args)
#     define SHOWBLKR(m, id) showblock(m, id)
#  else
#     define DBGPRTR(msg, args...)
#     define SHOWBLKR(m, id)
#  endif
#else
#  define DBGFLUSH
#  define DBGEOLN
#  define DBGPRT(msg, args...)
#  define SHOWBLK(m, id)
#  define DBGPRTM(msg, args...)
#  define SHOWBLKM(m, id)
#  define DBGPRTF(msg, args...)
#  define SHOWBLKF(m, id)
#  define DBGPRTR(msg, args...)
#  define SHOWBLKR(m, id)
#endif

/* This is intended to allow finding the header area from
   the address of the immediately adjacent memblocks.
   The guardxx avoid destruction by an off by one pointer
   and serve no real logical purpose.  Note that in some
   cases sz may be recovered from next or next may be
   recovered from sz.
*/
typedef struct memblock {
   struct memblock *prev;   /* 1st, protect against overrun */
   struct memblock *next;   /* makes this less clobberable  */
   ulong            sz;                 /* of this memblock */
   /* An allocated block has the next two (1) fields NULL */
   /* A free block has them both non-NULL  */
   struct memblock *nextfree;
   struct memblock *prevfree; /* actually data w/SAVEMEMORY */
#if SAVEMEMORY == 0
   ulong            guardlo;     /* may hold size requested */
#endif
   /* here lies the actual assigned storage                 */
   /* so the following must be addressed by adding offset   */
   /* storage must always be a multiple of 8 in size        */
   /* thus these items are fictional, i.e. for zero data    */
} memblock, *memblockp;

/* Notice that with SAVEMEMORY the prevfree field only
   exists for free blocks; it reuses data space.  This
   is why we cannot allow 0 sized blocks.
*/
#if SAVEMEMORY
#  define DATAOFFSET (offsetof(memblock, prevfree))
#else
#  define DATAOFFSET sizeof(memblock)
#endif

/* conversion and access macros */
#define MEMBLKp(p) (memblockp)((byte*)(p) - DATAOFFSET)
#define PTR(m)     (void*)((byte*)(m) + DATAOFFSET)

#define ALIGN 8
#define ALIGNMASK (ALIGN-1)

/* We can never use an allocation smaller than this */
#define MINSAVE   (ALIGN + DATAOFFSET)

/* Alternate form of NULL to distinguish free lists
   This is self protection, because freehdrs[1] is otherwise
   unused.  freehdrs[0] is reserved to hold lastsbrk.  In turn
   this means that ALIGN must be >= 4.
*/
#define NONE (memblockp)&freehdrs[1]

/* Magic constants.  MINSBRK must be MINSAVE or larger */
enum {NFLISTS = (int)(CHAR_BIT * sizeof(size_t)), MINSBRK = 1024};

/* ============== Globals ============= */

/* Headers of lists holding free blocks of 2**2 thru 2**31 size */
/* freehdr[n] holds items sized 2**n thru 2**(n+1) - 1          */

static memblockp freehdrs[NFLISTS]; /* yes, low indices are waste */
#define lastsbrk freehdrs[0]

/* keep track of the bases of each new sbrk block */
#define MAXSBRKS 100     /* I have never seen more than 5 needed */
static int   lastsbrkbgn;       /* zeroed on load */
static void *sbrkbgn[MAXSBRKS]; /* NULLS on load */

/* This holds pointers to hooks, initialized to NULLs */
/* see enum m_hook_kind for actual identifiers in sysquery.h */
static M_HOOKFN hookptr[HKCOUNT];

/* Forward declaration to allow sysquery init below */
static M_HOOKFN sethook(enum m_hook_kind which,
                        M_HOOKFN         newhook);

/* This allows a clean connection to debugging software */
static struct _sysquery sysquery = {
        DATAOFFSET,
#if SAVEMEMORY
        0xff,
#else
        offsetof(memblock, guardlo),
#endif
        offsetof(memblock, sz),
        offsetof(memblock, prevfree),
        offsetof(memblock, nextfree),
        offsetof(memblock, next),
        offsetof(memblock, prev),
        sizeof(memblock),
        NONE,         /* also &freehds[1] */
        &sbrkbgn,     /* anchors field */
        sethook       /* hookset field */
};

/* 1------------------1 */

/* This can return the above values, hopefully in a register */
/* The use of NONE in nextfree, prevfree may cause confusion */
struct _sysquery _sysmalloc(void)
{
   return sysquery;
} /* _sysmalloc */

/* 1------------------1 */

#if DEBUGM || DEBUGF || DEBUGR

/* These two routines are actually available in any user  */
/* application by use of the _sysmalloc call above.  They */
/* are retained here to show the derivation of user code, */
/* and in case needed during system initialization.       */

static void showblock(memblockp m, char *id)
{
   if (m) {
      DBGPRT(" %s %p sz=%u nxt=%p prv=%p nxtf=",
             id, m, m->sz, m->next, m->prev);
      if (m->nextfree) {
         if (NONE == m->nextfree)
            DBGPRT("NONE prvf=");
         else
            DBGPRT("%p prvf=", m->nextfree);
         if (NONE == m->prevfree)
            DBGPRT("NONE");
         else
            DBGPRT("%p", m->prevfree);
      }
      else DBGPRT("0");
   }
   else
      DBGPRT(" %s NULL", id);
} /* showblock */

/* 1------------------1 */

/* dump the entire free chain group */
static void dumpfree(void)
{
   int       i;
   memblockp m;

   for (i = 0; i < NFLISTS; i++) {
      if ((m = freehdrs[i])) {
         DBGPRT(EOL "%2d: ", i);
         do {
            DBGPRT("%p(%u)->", m, m->sz);
            m = m->nextfree;
         } while (m && (NONE != m));
         DBGPRT("0");
         m = freehdrs[i];
         while (m && (NONE !=m )) {
            SHOWBLK(m, EOL "     ");
            m = m->nextfree;
         }
      }
   }
   DBGEOLN;
} /* dumpfree */
#endif

/* 1------------------1 */

/* This is accessible only through the pointer    */
/* returned in the sysquery record by _sysmalloc. */
/* Only of use in the malldbg package.            */
/* No safeties implemented here - see malldbg     */
static M_HOOKFN sethook(enum m_hook_kind which,
                        M_HOOKFN         newhook)
{
   M_HOOKFN tmp = NULL;

   if (which < HKCOUNT) {
      tmp = hookptr[which];
      hookptr[which] = newhook;
   }
   return tmp;
} /* sethook */

/* 1------------------1 */

/* inserts bases of sbrk chains in sbrkbgn array  */
/* This ensures we can find all controlled memory */
/* gets called when we find an unexpected sbrk.   */
/* Note that if the sbrk was unaligned bk has now */
/* been aligned, and we have no record of wastage */
/* As long as nothing is returned to sbrk this is */
/* not a problem.  This only for the malldbg pkg. */
static void recordnewsbrk(memblockp bk)
{
   int i;

   if (lastsbrkbgn < MAXSBRKS - 1) {
      /* This check for a previous entry is probably not
         needed, but it is a rare occurance, so safety */
      for (i = 0; i < lastsbrkbgn; i++) {
         if (bk == sbrkbgn[i]) return;
      }
      sbrkbgn[lastsbrkbgn++] = bk;
   }
/* else we abandon trying to keep track */
} /* recordnewsbrk */

/* 1------------------1 */

static inline ulong roundup(size_t sz)
{
   ulong size;

   size = ((sz + ALIGNMASK) & ~ALIGNMASK) + DATAOFFSET;
   return size;
} /* roundup */

/* 1------------------1 */

static inline int size2bucket(ulong sz)
{
   int b;

   for (b = 0; sz; sz >>= 1, b++) continue;
   return b;
} /* size2bucket */

/* 1------------------1 */

static void badcallabort(const char *msg, int lgh, memblockp m)
{
#if DEBUGM || DEBUGF || DEBUGR
   DBGEOLN;
#endif
   write(STDERR, msg, lgh);
   write(STDERR, ": memory fouled\n", 16);
#if DEBUGM || DEBUGF || DEBUGR
   SHOWBLK(m, "");
   dumpfree();
#else
   (void)m;  /* anti unused warning */
#endif
   raise(SIGABRT);
} /* badcallabort */

/* 1------------------1 */

#define ISFREE(m) (m && (m != NONE) && m->nextfree && m->prevfree)
#if SAVEMEMORY
#define FOULED(m) (!lastsbrk || m->nextfree)
#else
#define FOULED(m) (!lastsbrk || (m->guardlo != 0xDEADBEEF))
#endif

/* 1------------------1 */

/* Unlike rmvfromfree, this extracts a block that */
/* may be buried deep within the free list by     */
/* unlinking.  m is already known a free block    */
static void extractfree(memblockp m)
{
   int       b;
   memblockp mnxtf, mprvf;

   if (m) {
      b = size2bucket(m->sz);
      SHOWBLKF(m, EOL "  extractfree blk");

      /* ease further tests */
      if (NONE == (mnxtf = m->nextfree)) m->nextfree = NULL;
      if (NONE == (mprvf = m->prevfree)) m->prevfree = NULL;

      if      (m->nextfree) m->nextfree->prevfree = mprvf;

      if      (m->prevfree) m->prevfree->nextfree = mnxtf;
      else if (m->nextfree) freehdrs[b] = mnxtf;
      else                  freehdrs[b] = NULL;

      /* mark the block non-free */
      m->nextfree = m->prevfree = NULL;

      DBGPRTF(EOL "  freehdrs %d", b);
      SHOWBLKF(freehdrs[b], "is blk");
   }
} /* extractfree */

/* 1------------------1 */

static inline memblockp combinelo(memblockp m)
{
   memblockp m1;

   m1 = m;
   if (ISFREE(m->prev)) {
      if (m->prev->next != m) {
         badcallabort("combinelo", 9, m);
         exit(EXIT_FAILURE);  /* prevent user trapping SIGABRT */
      }
      m1 = m->prev;
      extractfree(m1);
      if (NULL != (m1->next = m->next))
         m1->next->prev = m1;
      m1->sz += m->sz;
   }
   return m1;
} /* combinelo */

/* 1------------------1 */

/* used to combine with lastsbrk, so no ISFREE test */
/* because lastsbrk is not kept in the free lists   */
static memblockp combinehi(memblockp m)
{
   memblockp m1;

   if (m && m->next) {
      SHOWBLK(m,       EOL "  combinehi");
      SHOWBLK(m->next, EOL "  with");
      if (m->next->prev != m) {
         badcallabort("combinehi", 9, m);
         exit(EXIT_FAILURE);  /* prevent user trapping SIGABRT */
      }
      m1 = m->next;
      if (m1 != lastsbrk) extractfree(m1);
      if (NULL != (m->next = m->next->next))
         m->next->prev = m;
      m->sz += m1->sz;
      SHOWBLK(m,       EOL "  giving");
   }
   return m;
} /* combinehi */

/* 1------------------1 */

/* This takes care of marking the block as free */
static void mv2freelist(memblockp m)
{
   int       b;

   if (m) {
      if (ISFREE(m->next)) m = combinehi(m);
      b = size2bucket(m->sz);
      DBGPRT(EOL "  mv2freelist %d", b); SHOWBLK(m, "blk");
      if (lastsbrk && (m->next == lastsbrk)) {
         SHOWBLKF(lastsbrk, EOL "  Combine with lastsbrk");
         lastsbrk = combinehi(m);
         lastsbrk->nextfree = lastsbrk->prevfree = NULL;
         SHOWBLKF(lastsbrk, EOL "  Resulting in lastsbrk");
         return;
      }
      else if (freehdrs[b]) {
         m->nextfree = freehdrs[b];
         freehdrs[b]->prevfree = m;
      }
      else {
         m->nextfree = NONE;
      }
      m->prevfree = NONE;
      if (freehdrs[b]) freehdrs[b]->prevfree = m;
      freehdrs[b] = m;
      DBGPRT(EOL "  Exit mv2freelist");
   }
} /* mv2freelist */

/* 1------------------1 */

/* this always marks the block as non-free */
static inline void rmvfromfree(memblockp m)
{
   int b;

   if (m) {
      b = size2bucket(m->sz);
      DBGPRTM(EOL "  rmvfromfree %d", b); SHOWBLKM(m, "blk");
      if (m != freehdrs[b]) {
         DBGPRTM(" NOT FREE");
         badcallabort("rmvfromfree", 11, m);
         exit(EXIT_FAILURE);  /* prevent user trapping SIGABRT */
      }
      else {
         if (NONE == m->nextfree)
            freehdrs[b] = NULL;
         else {
            freehdrs[b] = m->nextfree;
            freehdrs[b]->prevfree = NONE;
         }
#if SAVEMEMORY
         m->nextfree = NULL;
#else
         m->nextfree = m->prevfree = NULL;
#endif
         DBGPRTM(EOL "  freehdrs %d", b);
         SHOWBLKM(freehdrs[b], "is blk");
      }
   }
} /* rmvfromfree */

/* 1------------------1 */

static int searchfree(ulong szneed)
{
   int b;

   b = size2bucket(szneed);
   DBGPRT(EOL "  freelist search from bucket %d", b);

   if (! freehdrs[b] || (freehdrs[b]->sz < szneed)) {
      do {
         b++;
      } while ((b < NFLISTS) && ! freehdrs[b]);
   }
   /* if found we will break off a piece and housekeep */
   if (b < NFLISTS)
      DBGPRT(", using %d", b);
   else {
      b = 0;
      DBGPRT(", none found");
   }
   return b;
} /* searchfree */

/* 1------------------1 */

/* The higher portion is returned in *mp,     */
/* the lower portion via the function return. */
/* and the lower portion is marked non-free   */
static memblockp split(memblockp *mp, ulong sz)
{
   memblockp m1, m;

   m = *mp;
   m1 = (memblockp)((char *)m + sz);
   if (m->sz < (sz + DATAOFFSET)) {
      badcallabort("memblockpsz", 11, m);
      exit(EXIT_FAILURE);  /* prevent user trapping SIGABRT */
   }
   memcpy(m1, m, DATAOFFSET);
   m1->prev = m;
   m1->sz = m->sz - sz;
   m->next = m1;
   m->sz = sz;
   m->nextfree = NULL;
#if SAVEMEMORY
#else
   m->prevfree = NULL;
#endif
   *mp = m1;
   if (m1->next) {
      if (m1->next->prev != m) {
         badcallabort("memblockpnxt", 12, m1);
         exit(EXIT_FAILURE);  /* prevent user trapping SIGABRT */
      }
      m1->next->prev = m1;
   }
   SHOWBLKM(m, EOL "  split returns");
   return m;
} /* split */

/* 1------------------1 */

/* Get the memory, see if it extends the present lastsbrk
   If not, put the old lastsbrk into the appropriate freelist
      and replace lastsbrk by the new, setting the headers up
   else update the size markers in lastsbrk.  When done either
   lastsbrk can supply the memory szextra, or NULL is returned.
   A revised lastsbrk block is marked as non-free.
*/
static memblockp extendsbrk(ulong szxtra)
{
   memblockp  m;
   byte      *expected;
   int        alignerr;
   int        aligndelta;

   DBGPRTM(", extending sbrk");

   /* we have to ensure that the new lastsbrk always has    */
   /* room to both realign and to leave a header when split */
   szxtra += (2 * ALIGN + DATAOFFSET);
   if (szxtra < MINSBRK) szxtra = MINSBRK;

   if (lastsbrk)
      expected = ((byte*)lastsbrk) + lastsbrk->sz;
   else expected = NULL;

   if ((aligndelta = (ulong)expected & ALIGNMASK)) {
      /* lastsbrk end was misaligned, try to align end of this */
      szxtra += ALIGN - aligndelta;
      aligndelta = 0;
   }

   m = fakesbrk(szxtra);
   if (-1 == (int)m) return NULL;
   else {
      if ((byte*)m == expected) {  /* Extending size of lastsbrk */
         DBGPRTM(EOL "  sbrk(%4u=0x%05x) got expected %p"
                     " lastsbrk %p sz %d",
                       szxtra, szxtra, expected,
                       lastsbrk, expected - (byte*)lastsbrk);
         lastsbrk->sz += szxtra;
         m = lastsbrk;
      }
      else {
         /* Here we have to check & fix alignment */
         DBGPRTM(EOL "=>sbrk(%4u=0x%05x) got UNEXPECTED %p/%p"
                     " lastsbrk %p sz %d",
                       szxtra, szxtra, m, expected,
                       lastsbrk, expected - (byte*)lastsbrk);
         if ((alignerr = (ALIGNMASK & (ulong)m))) {
/*f1*/      m = (memblockp)((char*)m +
                            (aligndelta = ALIGN - alignerr));
            DBGPRTM(", szerr %d/%d", aligndelta, alignerr);
         }
         m->sz = szxtra - aligndelta; /* discard alignerr bytes */
         m->prev = m->next = NULL;
#if SAVEMEMORY
         m->nextfree = NULL;
#else
         m->nextfree = m->prevfree = NULL;
         m->guardlo    = 0xDEADBEEF;
#endif
         mv2freelist(lastsbrk);
         lastsbrk = m;
         recordnewsbrk(m); /* save in list of chains */
      }
   }
   return m;
} /* extendsbrk */

/* 1------------------1 */

/* The mechanism:
   All available memory is kept on the free list, and all adjacent
   blocks, assigned or free, are linked by next/prev fields in order
   of address.  Freehdrs[n] holds the first of a list of free blocks
   of sizes between 2**n and 2**n+1. A pointer to the free portion
   of the block last acquired via sbrk is held in lastsbrk.

   All blocks on the freelist are marked by having a non-NULL value
   in the nextfree or prevfree fields.  The special value NONE is
   used to replace NULL to terminate these lists.  Because of the
   misalignment possibilities it is necessary to keep accurate byte
   count lengths in the sz component of lastsbrk.

   1.  An allocation is made from the first fit freehdrs list. Note
   that there MAY be a usable piece in the next lower freehdr, but
   that is ignored because we do not want to search possibly long
   lists.  The block is removed from the freelist, and any excess
   space is broken off (if large enough to be usable) and assigned
   to the appropriate free list.

   2.  If no suitable free block is found, allocation is attempted
   from the last block created by an sbrk call.  Such a block must
   be large enough to maintain an sbrk pointer after splitting off
   the desired allocation.

   3.  If this fails a new block is created (or extended) via an
   sbrk call.  If possible, the previous lastsbrk block is extended.
   If extension is not possible the remains of the old block alone
   is placed in the freelist.  This (non-extension) case results in
   the prev field of the lastsbrk block being NULL.  The next field
   of the lastsbrk block is always NULL. In this case only it is
   necessary to check and correct memory alignment.

   Insertion is always done into the start of any given freelist.
   Thus there is no list walking needed.  Similarly, any block is
   always removed from the head of the appropriate freelist.

   It is assumed that sbrk will never return a lower address than
   did a previous sbrk.  I am not sure if this affects anything. I
   believe it does not.
*/
void *nmalloc(size_t size)
{
   memblockp m = NULL, m1;
   ulong     szneed;
   int       b;
   void     *p = NULL;
   size_t    sz = size; /* preserve arg for hooks */

   /* compute the rounded up size needed */
   if (!sz) sz++;     /* avoid any 0 space allocation */
   szneed = roundup(sz);
   DBGPRTM("malloc(%5lu) [%5u]", sz, szneed);
   SHOWBLKM(lastsbrk, EOL "  lastsbrk");

   /* Check for oversize allocation request */
   if (szneed < ((ulong)(INT_MAX - 65536))) {
      /* search the free lists for one */
      b = searchfree(szneed);

      if (b) {
         rmvfromfree(m1 = freehdrs[b]);
         if (m1->sz < szneed + MINSAVE)
            m = m1;
         else {
            m = split(&m1, szneed);
            mv2freelist(m1);
         }
      }
      else if (lastsbrk &&
              (lastsbrk->sz >= (szneed + DATAOFFSET))) {
         m = split(&lastsbrk, szneed);
      }
      /* if not found get more from system */
      else if ((m1 = extendsbrk(szneed))) {
         if (m1->sz < szneed + MINSAVE) {
            m = m1;
            DBGPRTM(EOL "**FOULED lastsbrk\a");
         }
         else {
            m = split(&lastsbrk, szneed);
         }
      }
   /* else abject_failure(); */
   /* abject_failure COULD check the first possible freehdrs */
   /* list as a last chance to find some suitable memory     */

      if (m) p = PTR(m);
      else {
         DBGPRTM(dbgbp, ", FAILURE");
         p = NULL;
      }
   }
/* else m and p are NULL for oversize; */

#if DEBUGM
   DBGPRTM(EOL "returns %p", p);
   if (m) DBGPRTM("(%lu)", m->sz - DATAOFFSET);
   DBGEOLN;
#endif

   if (hookptr[malloc_HK]) hookptr[malloc_HK](size, p);
   if (!p && hookptr[malloc_fail_HK])
      hookptr[malloc_fail_HK](size, NULL);

   return p;
} /* nmalloc */

/* 1------------------1 */

static void dofree(memblockp m)
{
   /* mark the block free */
   m->nextfree = m->prevfree = NONE;

   /* try to combine with lower or higher blocks in memory */
   if (ISFREE(m->next)) m = combinehi(m);
   if (ISFREE(m->prev)) m = combinelo(m);

   if (lastsbrk && (lastsbrk == m->prev) )
      DBGPRTF(EOL "**Found decreasing sbrk!! FOUL");
   else mv2freelist(m);
} /* dofree */

/* 1------------------1 */

void nfree(void *ptr)
{
   memblockp m;

   if (hookptr[free_HK]) hookptr[free_HK](0, ptr);

   if (ptr) {
      m = MEMBLKp(ptr);
      DBGPRTF("free(%p)", ptr); SHOWBLKF(m, "");
      if (ISFREE(m) ||     /* bad, refreeing block */
          FOULED(m) ) {    /* block is fouled */
         badcallabort("free", 4, m);
         return;           /* he can trap this SIGABRT */
      }
      dofree(m);
#if DEBUGF
      DBGEOLN;
#endif
   }
   else if (hookptr[free_null_HK])
      hookptr[free_null_HK](0, NULL);
} /* nfree */

/* 1------------------1 */

static memblockp mv2lastsbrk(memblockp m, ulong szneed)
{
   memblockp m1;

   m1 = split(&lastsbrk, szneed);

   /* Now m1 is the proposed new block, of the right size */
   /* links are already revised so copy data from m to m2 */
   memcpy(PTR(m1), PTR(m), m->sz - DATAOFFSET);

   dofree(m);
   return m1;
} /* mv2lastsbrk */

/* 1------------------1 */

void *nrealloc(void *ptr, size_t size)
{
   memblockp m, m1, m2;
   void     *p;
   ulong     szneed;
   int       b;
   size_t    sz = size;

   if (hookptr[realloc_HK]) hookptr[realloc_HK](sz, ptr);

   if (!ptr) {
      p = nmalloc(sz);
      if (hookptr[realloc_exit_HK])
         hookptr[realloc_exit_HK](size, p);
      return p;
   }

   m = m1 = MEMBLKp(ptr);
   if (!sz) sz++;     /* avoid any 0 space allocation */
   szneed = roundup(sz);
   DBGPRTR("realloc(%p:%lu[%u])", ptr, sz, szneed);
   SHOWBLKR(m, EOL "  was");
   if (ISFREE(m) ||     /* bad, realloc of free block */
       FOULED(m) ) {    /* storage fouled */
      badcallabort("realloc", 7, m);
      p = NULL;
      goto exeunt;      /* he can trap this SIGABRT */
   }
   SHOWBLKR(lastsbrk, EOL "  lastsbrk");

   /* if decreasing simply reduce size and move excess to free */
   if (szneed <= m->sz) {
      DBGPRTR(EOL "  Realloc is reducing");
      if ((m->sz - szneed) >= MINSAVE) {
         m = split(&m1, szneed);
         mv2freelist(m1);
      }
   /* else just return old pointer, i.e. NOP */
   }
   else if (szneed > ((ulong)(INT_MAX - 65536))) {
      /* reject excessive size request */
      p = NULL; goto exeunt;
   }
   else if (ISFREE(m->next) &&
            (szneed <= (m->sz + m->next->sz)) ) {
      /* the 'next' block is free and adequate so use it */
      DBGPRTR(EOL "  Realloc is combining, next is free");
      m = m1 = combinehi(m);
      /* now split off the excess, if any */
      if ((m->sz - szneed) >= MINSAVE) {
         m = split(&m1, szneed);
         mv2freelist(m1);
      }
   /* else m is the oversized return block */
   }
   else if ((lastsbrk == m->next)  &&
            ((szneed + MINSAVE) <= (m->sz + lastsbrk->sz)) ) {
      /* lastsbrk is adequate and adjacent so use it */
      DBGPRTR(EOL "  Realloc is using lastsbrk to extend");
      m = m1 = combinehi(m1);
      m = split(&m1, szneed);
      lastsbrk = m1;
   }
   else if (ISFREE(m->prev) &&
            (szneed <= (m->sz + m->prev->sz)) ) {
      /* the 'prev' block is free and adequate so use it */
      DBGPRTR(EOL "  Realloc is combining low free, moving data");
      m1 = m->prev;
      extractfree(m1);
      m1->sz += m->sz;     /* revise the links */
      if ((m1->next = m->next)) m1->next->prev = m1;
      /* we are now done with m links, except sz */

      /* This involves copying the data, overlapping */
      memmove(PTR(m1), PTR(m), m->sz - DATAOFFSET);

      m = m1;        /* done with the old m value */
      /* Is there something leftover */
      if ((m->sz - szneed) >= MINSAVE) {
         m = split(&m1, szneed);
         mv2freelist(m1);
      }
   }
   else if ((b = searchfree(szneed))) {
      /* An adequate free block exists, copy over, free old */
      DBGPRTR(EOL "  Realloc is using free block, copying");
      rmvfromfree(m1 = freehdrs[b]);
      if (m1->sz < szneed + MINSAVE) {
         m2 = m1;
      }
      else {
         m2 = split(&m1, szneed);
         mv2freelist(m1);
      }
      /* Now m2 is the proposed new block, of the right size */
      /* links are already revised so copy data from m to m2 */
      memcpy(PTR(m2), PTR(m), m->sz - DATAOFFSET);

      dofree(m);
      m = m2;
   }
   else if (lastsbrk &&
            ((szneed + MINSAVE) <= lastsbrk->sz) ) {
      DBGPRTR(EOL "  Realloc is copying into lastsbrk");
      m = mv2lastsbrk(m, szneed);
   }
   /* else malloc new size, copy data, and free old */
   else if ((m1 = extendsbrk(szneed))) {
      if (lastsbrk == m->next) {
         DBGPRTR(EOL "  Realloc is now using lastsbrk extended");
         /* last chance to avoid copying */
         m = m1 = combinehi(m);
         m = split(&m1, szneed);
         lastsbrk = m1;
      }
      else {
         /* At this point lastsbrk is adequate size */
         /* split off, copy over, and free old      */
         DBGPRTR(EOL "  Realloc is making complete new copy");
         m = mv2lastsbrk(m, szneed);
      }
   }
   else m = NULL;  /* failure */

   if (m) p = PTR(m);
   else {
      DBGPRTR(dbgbp, ", FAILURE");
      p = NULL;
   }

#if DEBUGR
   DBGPRTR(EOL "returns %p", p);
   if (m) DBGPRTR("(%lu)", m->sz - DATAOFFSET);
   DBGEOLN;
#endif

exeunt:       /* label used on realloc of free block */
              /* and on trap of oversize request */
   if (!p && ptr && hookptr[malloc_fail_HK])
      hookptr[malloc_fail_HK](size, ptr);
   if (hookptr[realloc_exit_HK])
      hookptr[realloc_exit_HK](size, p);

   return p;
} /* nrealloc */

/* 1------------------1 */

/* calloc included here to ensure that it handles the
   same range of sizes (s * n) as does malloc.  The
   multiplication n*s can wrap, yielding a too small
   value, so we must ensure calloc rejects this.
*/
void *ncalloc(size_t n, size_t s)
{
   void   *result;
   size_t  sz;

   result = NULL;
   if (!n || (((size_t)-1) / n) > s) {
      sz = n * s;
      if ((result = nmalloc(sz))) memset(result, 0, sz);
      }
   return result;
} /* ncalloc */

/* 1------------------1 */

/* The remaining code is an attempt to graft on the
   memalign function.  It can do with improvement.
   The idea is to do this without disturbing the
   already checked and debugged package.

   In units of ALIGN (== 8 here) bytes.  The value of
   DATAOFFSET depends on SAVEMEMORY, either 2 or 3.  The
   following assumes SAVEMEMORY is set and DATAOFFSET = 2.

   Initial malloc block (alignment > ALIGN):
    ____________________________________________________
   |          |            |              |             |
   |          |            |              |             |
   |DATAOFFSET|   size     |  xtra for    |             |
   | control  |            | realignment  |             |
   |__________|____________|______________|_____________|
              ^
              ^
A: If this point is aligned, then we simply cut the assignment.
   Immediate use of realloc will avoid any data movement.

B: Worst case when this is 1 (DELTA) above an alignment point.
   becomes (since alignment > ALIGN and thus
                  alignment >= DATAOFFSET):
    ____________________________________________________
   |          |            |              |             |
   |alignment |DATAOFFSET  |              |     0       |
   | - DELTA  |   for      |    size      |  nothing    |
   |to freespc| control    |              | needed BUT: |
   |________ _|____________|______________|_____________|
                           ^              ^
                           ^              ^
   Now this point is suitably aligned     ^
    ALIGNMENT - DELTA >= DATAOFFSET       ^
                                          ^
   For the intermediate cases make sure this block to be moved
   to free space is at least 3 units long.  Easiest is to set
   the initial extra value so that this worst case is 3, not 0
 ?  This makes the initial extra size alignment+4 ??

C: The initial alignment point is 1 low.  (DELTA = -1)
    ____________________________________________________
   |          |            |              |             |
   |    3     |            |              |             |
   |control+1 |    2       |    size      |  xtra-3     |
   |to freespc|  control   |              |to freespc   |
   |__________|____________|______________|_____________|
                           ^              ^
                           ^              ^
   This point is aligned --^     This goes to freespace

D: The initial alignment point is 2 low.  (DELTA = -2)
    ____________________________________________________
   |          |            |              |             |
   |          |            |              |             |
   |          |    2       |    size      |             |
   |          |  control   |              |to freespc   |
   |__________|____________|______________|_____________|
                           ^              ^
                           ^              ^
   This point is aligned --^     This goes to freespace

   The minimum of 3 units to freespace is because 2 are
   required for control, and without further space the
   block is useless.  In fact it needs the further space
   to implement the free block linking mechanism.
*/

/* 1------------------1 */

/* Check alignment is a non-zero power of two <= 65536. */
/* Return 0 if so, else non-zero                        */
static inline int invalid(size_t alignment)
{
   if (alignment && (alignment <= 65536))
      return (alignment & (alignment - 1));
   else return 1;  /* 0 is invalid */
} /* invalid */

/* 1------------------1 */

/* define the XTRA storage needed to assure chopping up feasible
   and that chopped off storage is large enough to be usable.
   XTRA is always a multiple of ALIGN.
*/

#define XTRA (alignment + 3 * ALIGN)

/* 1------------------1 */

/* return memory aligned so that the return value is a */
/* multiple of alignment.  Otherwise similar to malloc */
/* alignment MUST be a power of two, max 65536.        */

void *nmemalign(size_t alignment, size_t size)
{
   memblockp m = NULL;
   void     *minit;
   ulong     misalign;
   size_t    szneed, sz = size; /* preserve arg for hooks */

   /* compute the rounded up size needed */
   if (!sz) sz++;     /* avoid any 0 space allocation */
   szneed = roundup(sz);
   DBGPRTM("memalign(%5lu) [%5lu] %5lu", sz, szneed, alignment);
   DBGEOLN;

   if (size < ((ulong)(INT_MAX - 65536)) &&
       !invalid(alignment)) {
      /* parameters seem to be valid */
      if (alignment <= ALIGN) {
         DBGPRTM("  alignment value ignored"); DBGEOLN;
         return nmalloc(szneed);                      /* EXIT */
      }
      else if ((minit = nmalloc(szneed + XTRA))) {
         /* alignment >= 2*ALIGN and power of 2 if here */
         misalign = (ulong)minit % alignment;
         DBGPRTM("  misalignment = %d", misalign);
         if (0 == misalign) { /* aligned, just return XTRA */
            DBGPRTM(" Just realloc the block."); DBGEOLN;
            return nrealloc(minit, size);             /* EXIT */
         }
         else {
            /* two or more chunks to release */
            /* for now, just return NULL and have a leak */
            DBGPRTM("  Complex case, release multiple chunks");
            DBGEOLN;
         }
      } /* alignment > ALIGN */
   } /* valid parameters */
   if (m) return PTR(m);
   else return NULL;
} /* nmemalign */

/* --------- nmalloc.c ----------- */
