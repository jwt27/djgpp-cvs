/* file cokusMT.h */

#ifndef cokus_h
#define cokus_h

#include <limits.h>

#if ULONG_MAX != 4294967295UL
   #error System long word size not suitable for cokusMT
#endif

#define ranMTMAX ULONG_MAX

void seedMT(unsigned long seed);
unsigned long randomMT(void);

#endif /* cokus_h */
