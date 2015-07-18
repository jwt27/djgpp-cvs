
/* @(#)w_log10.c 5.1 93/09/24 */
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
	<<log2>>, <<log2f>>---base 2 logarithms

INDEX
log2
INDEX
log2f

ANSI_SYNOPSIS
	#include <math.h>
	double log2(double <[x]>);
	float log2f(float <[x]>);

TRAD_SYNOPSIS
	#include <math.h>
	double log2(<[x]>)
	double <[x]>;

	float log2f(<[x]>)
	float <[x]>;

DESCRIPTION
<<log2>> returns the base 2 logarithm of <[x]>.
It is implemented as <<log(<[x]>) / log(2)>>.

<<log2f>> is identical, save that it takes and returns <<float>> values.

RETURNS
<<log2>> and <<log2f>> return the calculated value. 

See the description of <<log>> for information on errors.

PORTABILITY
<<log2>> is ANSI C.  <<log2f>> is an extension.

 */

/* 
 * wrapper log2(X)
 */

#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double log2(double x)		/* wrapper log2 */
#else
	double log2(x)			/* wrapper log2 */
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_log2(x);
#else
	double z;
	z = __ieee754_log2(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(x<=0.0) {
	    if(x==0.0)
	        return __kernel_standard(x,x,27); /* log2(0) */
	    else 
	        return __kernel_standard(x,x,28); /* log2(x<0) */
	} else
	    return z;
#endif
}

#endif /* defined(_DOUBLE_IS_32BITS) */
