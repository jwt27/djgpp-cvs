/* wf_atan2.c -- float version of w_atan2.c.
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
 * wrapper atan2f(y,x)
 */

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float atan2f(float y, float x)		/* wrapper atan2f */
#else
	float atan2f(y,x)			/* wrapper atan2 */
	float y,x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_atan2f(y,x);
#else
	_float_long_union ux;
	_float_long_union uy;
	float z;
	
	ux.f = x;
	uy.f = y;

	z = __ieee754_atan2f(y,x);
	if(_LIB_VERSION == _IEEE_||isnanf(ux.l)||isnanf(uy.l)) return z;
	if(x==(float)0.0&&y==(float)0.0) {
		/* atan2f(+-0,+-0) */
	        return (float)__kernel_standard((double)y,(double)x,103);
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double atan2(double y, double x)
#else
	double atan2(y,x)
	double y,x;
#endif
{
	return (double) atan2f((float) y, (float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
