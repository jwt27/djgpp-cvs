/* -*-C-*- store.c */

#include "elefunt.h"

void
store(p)
float* p;
{
   /* NO-OP -- want to force memory store of argument */
   /* Needed for IEEE arithmetic plus Honeywell (floating point */
   /* registers larger than memory word size */
}
