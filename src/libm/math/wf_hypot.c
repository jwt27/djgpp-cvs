/* wf_hypot.c -- float version of w_hypot.c.
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
 * wrapper hypotf(x,y)
 */

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float hypotf(float x, float y)	/* wrapper hypotf */
#else
	float hypotf(x,y)		/* wrapper hypotf */
	float x,y;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_hypotf(x,y);
#else
	_float_long_union ux;
	_float_long_union uy;
	_float_long_union uz;
	float z;
	
	ux.f = x;
	uy.f = y;

	z = __ieee754_hypotf(x,y);
	uz.f = z;
	if(_LIB_VERSION == _IEEE_) return z;
	if((!finitef(uz.l))&&finitef(ux.l)&&finitef(uy.l))
	    /* hypot overflow */
	    return (float)__kernel_standard((double)x,(double)y,104);
	else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double hypot(double x, double y)
#else
	double hypot(x,y)
	double x,y;
#endif
{
	return (double) hypotf((float) x, (float) y);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
