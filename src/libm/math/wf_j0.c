/* wf_j0.c -- float version of w_j0.c.
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
 * wrapper j0f(float x), y0f(float x)
 */

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float j0f(float x)		/* wrapper j0f */
#else
	float j0f(x)			/* wrapper j0f */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_j0f(x);
#else
	_float_long_union ux;
	float z;
	ux.f = x;

	z = __ieee754_j0f(x);
	if(_LIB_VERSION == _IEEE_ || isnanf(ux.l)) return z;
	if(fabsf(x)>(float)X_TLOSS) {
		/* j0f(|x|>X_TLOSS) */
	        return (float)__kernel_standard((double)x,(double)x,134);
	} else
	    return z;
#endif
}

#ifdef __STDC__
	float y0f(float x)		/* wrapper y0f */
#else
	float y0f(x)			/* wrapper y0f */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_y0f(x);
#else
	_float_long_union ux;
	float z;
	
	ux.f = x;

	z = __ieee754_y0f(x);
	if(_LIB_VERSION == _IEEE_ || isnanf(ux.l) ) return z;
        if(x <= (float)0.0){
                if(x==(float)0.0)
                    /* d= -one/(x-x); */
                    return (float)__kernel_standard((double)x,(double)x,108);
                else
                    /* d = zero/(x-x); */
                    return (float)__kernel_standard((double)x,(double)x,109);
        }
	if(x>(float)X_TLOSS) {
		/* y0(x>X_TLOSS) */
	        return (float)__kernel_standard((double)x,(double)x,135);
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double j0(double x)
#else
	double j0(x)
	double x;
#endif
{
	return (double) j0f((float) x);
}

#ifdef __STDC__
	double y0(double x)
#else
	double y0(x)
	double x;
#endif
{
	return (double) y0f((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
