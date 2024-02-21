/* -*-C-*- randl.c */

#include "elefunt.h"

/***********************************************************************

Returns pseudo  random numbers  logarithmically distributed  over
(1,exp(x)).  Thus a*randl(ln(b/a)) is logarithmically distributed
in (a,b).

Other subroutines required:

	exp(x) - The exponential routine.

	ran(k) - A function program returning random real
		numbers uniformly distributed over (0,1).  The
		argument k is a dummy.

***********************************************************************/

float
randl(x)
float x;
{

    static int k = 1;

    return (exp(x * ran(k)));
}
