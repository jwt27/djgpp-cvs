/* wf_acosh.c -- float version of w_acosh.c.
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
 * wrapper acoshf(x)
 */

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float acoshf(float x)		/* wrapper acoshf */
#else
	float acoshf(x)			/* wrapper acoshf */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_acoshf(x);
#else
	_float_long_union ux;
	float z;
	
	ux.f = x;

	z = __ieee754_acoshf(x);
	if(_LIB_VERSION == _IEEE_ || isnanf(ux.l)) return z;
	if(x<(float)1.0) {
		/* acosh(x<1) */
	        return (float)__kernel_standard((double)x,(double)x,129);
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double acosh(double x)
#else
	double acosh(x)
	double x;
#endif
{
	return (double) acoshf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
