/* -------- nmalloc.h ----------- */

/* Copyright (c) 2003, 2006 by Charles B. Falconer
   Licensed under the terms of the GNU LIBRARY GENERAL PUBLIC
   LICENSE and/or the terms of COPYING.DJ, all available at
   <http://www.delorie.com>.

   Bug reports to <mailto:djgpp@delorie.com>
*/

#ifndef nmalloc_h
#define nmalloc_h

#include <stddef.h>

void *nmalloc(size_t sz);
void nfree(void *ptr);
void *nrealloc(void *ptr, size_t sz);
void *ncalloc(size_t n, size_t s);
#ifdef MEMALIGN
void *nmemalign(size_t alignment, size_t sz);
#endif

#endif
/* -------- nmalloc.h ----------- */
