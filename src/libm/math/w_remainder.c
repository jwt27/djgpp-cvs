
/* @(#)w_remainder.c 5.1 93/09/24 */
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
<<rint>>, <<rintf>>, <<rintl>>, <<remainder>>, <<remainderf>>---round and  remainder
INDEX
	rint
INDEX
	rintf
INDEX
	rintl
INDEX
	remainder
INDEX
	remainderf

ANSI_SYNOPSIS
	#include <math.h>
	double rint(double <[x]>);
	float rintf(float <[x]>);
	long double rintl(long double <[x]>);
	double remainder(double <[x]>, double <[y]>);
	float remainderf(float <[x]>, float <[y]>);

TRAD_SYNOPSIS
	#include <math.h>
	double rint(<[x]>)
	double <[x]>;
	float rintf(<[x]>)
	float <[x]>;
	long double rint(<[x]>)
	long double <[x]>;
	double remainder(<[x]>,<[y]>)
	double <[x]>, <[y]>;
	float remainderf(<[x]>,<[y]>)
	float <[x]>, <[y]>;

DESCRIPTION
<<rint>>, <<rintf>> and <<rintl>> returns their argument rounded to the nearest
integer.  <<remainder>> and <<remainderf>> find the remainder of
<[x]>/<[y]>; this value is in the range -<[y]>/2 .. +<[y]>/2.

RETURNS
<<rint>> and <<remainder>> return the integer result as a double.

PORTABILITY
<<rint>> and <<remainder>> are System V release 4.  <<rintf>>, <<rintl>> and
<<remainderf>> are extensions.

*/

/* 
 * wrapper remainder(x,p)
 */

#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double remainder(double x, double y)	/* wrapper remainder */
#else
	double remainder(x,y)			/* wrapper remainder */
	double x,y;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_remainder(x,y);
#else
	double z;
	z = __ieee754_remainder(x,y);
	if(_LIB_VERSION == _IEEE_ || isnan(y)) return z;
	if(y==0.0) 
	    return __kernel_standard(x,y,28); /* remainder(x,0) */
	else
	    return z;
#endif
}

#endif /* defined(_DOUBLE_IS_32BITS) */
