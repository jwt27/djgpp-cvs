/* wf_remainder.c -- float version of w_remainder.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

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
 * wrapper remainderf(x,p)
 */

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float remainderf(float x, float y)	/* wrapper remainder */
#else
	float remainderf(x,y)			/* wrapper remainder */
	float x,y;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_remainderf(x,y);
#else
	_float_long_union uy;
	float z;
	
	uy.f = y;

	z = __ieee754_remainderf(x,y);
	if(_LIB_VERSION == _IEEE_ || isnanf(uy.l)) return z;
	if(y==(float)0.0) 
	    /* remainder(x,0) */
	    return (float)__kernel_standard((double)x,(double)y,128);
	else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double remainder(double x, double y)
#else
	double remainder(x,y)
	double x,y;
#endif
{
	return (double) remainderf((float) x, (float) y);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
