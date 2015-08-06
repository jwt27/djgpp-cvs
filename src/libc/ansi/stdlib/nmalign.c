/* Copyright (c) 2003, 2007 by Charles B. Falconer
   Licensed under the terms of the GNU LIBRARY GENERAL PUBLIC
   LICENSE and/or the terms of COPYING.DJ, all available at
   <http://www.delorie.com>.

   Bug reports to <mailto:djgpp@delorie.com>

   Revised 2007-01-04 to include calloc.
   Revised 2007-01-28 per bug report by Florian Xaver. *f1*
   Revised 2014, 2015 for use in DJGPP libc by Andris Pavenis <andris.pavenis@iki.fi>
*/

#include "nmalcdef.h"

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

