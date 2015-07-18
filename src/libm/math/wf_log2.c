/* wf_log2.c -- float version of w_log2.c.
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
 * wrapper log2f(X)
 */

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float log2f(float x)		/* wrapper log2f */
#else
	float log2f(x)			/* wrapper log2f */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_log2f(x);
#else
	_float_long_union ux;
	float z;
	
	ux.f = x;

	z = __ieee754_log2f(x);
	if(_LIB_VERSION == _IEEE_ || isnanf(ux.l)) return z;
	if(x<=(float)0.0) {
	    if(x==(float)0.0)
	        /* log2(0) */
	        return (float)__kernel_standard((double)x,(double)x,127);
	    else 
	        /* log2(x<0) */
	        return (float)__kernel_standard((double)x,(double)x,128);
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double log2(double x)
#else
	double log2(x)
	double x;
#endif
{
	return (double) log2f((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
