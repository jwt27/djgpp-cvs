/* wf_log.c -- float version of w_log.c.
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
 * wrapper logf(x)
 */

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float logf(float x)		/* wrapper logf */
#else
	float logf(x)			/* wrapper logf */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_logf(x);
#else
	_float_long_union ux;
	float z;
	
	ux.f = x;

	z = __ieee754_logf(x);
	if(_LIB_VERSION == _IEEE_ || isnanf(ux.l) || x > (float)0.0) return z;
	if(x==(float)0.0)
	    /* logf(0) */
	    return (float)__kernel_standard((double)x,(double)x,116);
	else 
	    /* logf(x<0) */
	    return (float)__kernel_standard((double)x,(double)x,117);
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double log(double x)
#else
	double log(x)
	double x;
#endif
{
	return (double) logf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
