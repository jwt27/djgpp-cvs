
/* @(#)s_ldexp.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/*
FUNCTION
       <<ldexp>>, <<ldexpf>>---load exponent

INDEX
	ldexp
INDEX
	ldexpf

ANSI_SYNOPSIS
       #include <math.h>
       double ldexp(double <[val]>, int <[expon]>);
       float ldexpf(float <[val]>, int <[expon]>);

TRAD_SYNOPSIS
       #include <math.h>

       double ldexp(<[val]>, <[expon]>)
              double <[val]>;
              int <[expon]>;

       float ldexpf(<[val]>, <[expon]>)
              float <[val]>;
              int <[expon]>;


DESCRIPTION
<<ldexp>> calculates the value 
@ifinfo
<[val]> times 2 to the power <[expon]>.
@end ifinfo
@tex
$val\times 2^{expon}$.
@end tex
<<ldexpf>> is identical, save that it takes and returns <<float>>
rather than <<double>> values.

RETURNS
<<ldexp>> returns the calculated value.

Underflow and overflow both set <<errno>> to <<ERANGE>>.
On underflow, <<ldexp>> and <<ldexpf>> return 0.0.
On overflow, <<ldexp>> returns plus or minus <<HUGE_VAL>>.

PORTABILITY
<<ldexp>> is ANSI, <<ldexpf>> is an extension.
              
*/   

#include "fdlibm.h"
#include <errno.h>
#include <float.h>

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double ldexp(double value, int expon)
#else
	double ldexp(value, expon)
	double value; int expon;
#endif
{
	if(!finite(value)||value==0.0) return value;
	value = scalbn(value,expon);
	if(!finite(value)||
	   (value < DBL_MIN && value > -DBL_MIN)) errno = ERANGE;
	return value;
}

#endif /* _DOUBLE_IS_32BITS */
