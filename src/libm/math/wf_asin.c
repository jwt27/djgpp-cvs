/* wf_asin.c -- float version of w_asin.c.
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
 *
 */

/* 
 * wrapper asinf(x)
 */


#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float asinf(float x)		/* wrapper asinf */
#else
	float asinf(x)			/* wrapper asinf */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_asinf(x);
#else
	_float_long_union ux;
	float z;
	
	ux.f = x;

	z = __ieee754_asinf(x);
	if(_LIB_VERSION == _IEEE_ || isnanf(ux.l)) return z;
	if(fabsf(x)>(float)1.0) {
	    /* asinf(|x|>1) */
	    return (float)__kernel_standard((double)x,(double)x,102);
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double asin(double x)
#else
	double asin(x)
	double x;
#endif
{
	return (double) asinf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
