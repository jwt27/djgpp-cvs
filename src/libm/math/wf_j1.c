/* wf_j1.c -- float version of w_j1.c.
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
 * wrapper of j1f,y1f 
 */

#include "fdlibm.h"
#include <libc/ieee.h>


#ifdef __STDC__
	float j1f(float x)		/* wrapper j1f */
#else
	float j1f(x)			/* wrapper j1f */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_j1f(x);
#else
	_float_long_union ux;
	float z;
	
	ux.f = x;

	z = __ieee754_j1f(x);
	if(_LIB_VERSION == _IEEE_ || isnanf(ux.l) ) return z;
	if(fabsf(x)>(float)X_TLOSS) {
		/* j1(|x|>X_TLOSS) */
	        return (float)__kernel_standard((double)x,(double)x,136);
	} else
	    return z;
#endif
}

#ifdef __STDC__
	float y1f(float x)		/* wrapper y1f */
#else
	float y1f(x)			/* wrapper y1f */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_y1f(x);
#else
	_float_long_union ux;
	float z;
	
	ux.f = x;

	z = __ieee754_y1f(x);
	if(_LIB_VERSION == _IEEE_ || isnanf(ux.l) ) return z;
        if(x <= (float)0.0){
                if(x==(float)0.0)
                    /* d= -one/(x-x); */
                    return (float)__kernel_standard((double)x,(double)x,110);
                else
                    /* d = zero/(x-x); */
                    return (float)__kernel_standard((double)x,(double)x,111);
        }
	if(x>(float)X_TLOSS) {
		/* y1(x>X_TLOSS) */
	        return (float)__kernel_standard((double)x,(double)x,137);
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double j1(double x)
#else
	double j1(x)
	double x;
#endif
{
	return (double) j1f((float) x);
}

#ifdef __STDC__
	double y1(double x)
#else
	double y1(x)
	double x;
#endif
{
	return (double) y1f((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
