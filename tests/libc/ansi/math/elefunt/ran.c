/* -*-C-*- ran.c */

#include "elefunt.h"

/***********************************************************************
Random number generator - based on Algorithm 266 by Pike and Hill
(modified by Hansson), Communications of the ACM, Vol. 8, No. 10,
October 1965.

This subprogram is intended for use on computers with fixed point
wordlength of at least 29 bits.  It is best if the floating point
significand has at most 29 bits.
***********************************************************************/

float
ran(k)
int k;					/* unused argument */
{
    static long iy = 100001L;

    iy = iy * 125;
    iy = iy - (iy / 2796203L) * 2796203L;
    return (((float) (iy)) / 2796203.0e0);
}
