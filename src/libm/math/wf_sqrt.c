/* wf_sqrt.c -- float version of w_sqrt.c.
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
 * wrapper sqrtf(x)
 */

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float sqrtf(float x)		/* wrapper sqrtf */
#else
	float sqrtf(x)			/* wrapper sqrtf */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_sqrtf(x);
#else
	_float_long_union ux;
	float z;
	
	ux.f = x;

	z = __ieee754_sqrtf(x);
	if(_LIB_VERSION == _IEEE_ || isnanf(ux.l)) return z;
	if(x<(float)0.0) {
	    /* sqrtf(negative) */
	    return (float)__kernel_standard((double)x,(double)x,126);
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double sqrt(double x)
#else
	double sqrt(x)
	double x;
#endif
{
	return (double) sqrtf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
