/* wf_fmod.c -- float version of w_fmod.c.
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
 * wrapper fmodf(x,y)
 */

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float fmodf(float x, float y)	/* wrapper fmodf */
#else
	float fmodf(x,y)		/* wrapper fmodf */
	float x,y;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_fmodf(x,y);
#else
	_float_long_union ux;
	_float_long_union uy;
	float z;
	
	ux.f = x;
	uy.f = y;

	z = __ieee754_fmodf(x,y);
	if(_LIB_VERSION == _IEEE_ ||isnanf(uy.f)||isnanf(ux.f)) return z;
	if(y==(float)0.0) {
		/* fmodf(x,0) */
	        return (float)__kernel_standard((double)x,(double)y,127);
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double fmod(double x, double y)
#else
	double fmod(x,y)
	double x,y;
#endif
{
	return (double) fmodf((float) x, (float) y);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
