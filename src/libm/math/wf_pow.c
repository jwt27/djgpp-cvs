/* wf_pow.c -- float version of w_pow.c.
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
 * wrapper powf(x,y) return x**y
 */
#include <float.h>
#include <libc/ieee.h>
#include "fdlibm.h"

#ifdef __STDC__
	float powf(float x, float y)	/* wrapper powf */
#else
	float powf(x,y)			/* wrapper powf */
	float x,y;
#endif
{
#ifdef _IEEE_LIBM
	return  __ieee754_powf(x,y);
#else
	_float_long_union ux;
	_float_long_union uy;
	_float_long_union uz;
	float z;

	ux.f = x;
	uy.f = y;

	z=__ieee754_powf(x,y);
	uz.f = z;
	if(_LIB_VERSION == _IEEE_|| isnanf(uy.l)) return z;
	if(isnanf(ux.l)) {
	    if(y==(float)0.0) 
	        /* powf(NaN,0.0) */
	        return (float)__kernel_standard((double)x,(double)y,142);
	    else 
		return z;
	}
	if(x==(float)0.0){ 
	    if(y==(float)0.0)
	        /* powf(0.0,0.0) */
	        return (float)__kernel_standard((double)x,(double)y,120);
	    if(finitef(uy.l)&&y<(float)0.0)
	        /* powf(0.0,negative) */
	        return (float)__kernel_standard((double)x,(double)y,123);
	    return z;
	}
	if(!finitef(uz.l )) {
	    if(finitef(ux.l)&&finitef(uy.l)) {
	        if(isnanf(uz.l))
		    /* powf neg**non-int */
	            return (float)__kernel_standard((double)x,(double)y,124);
	        else 
		    /* powf overflow */
	            return (float)__kernel_standard((double)x,(double)y,121);
	    }
	} 
	if(z < FLT_MIN &&finitef(ux.l)&&finitef(uy.l))
	    /* powf underflow */
	    return (float)__kernel_standard((double)x,(double)y,122);
	return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double pow(double x, double y)
#else
	double pow(x,y)
	double x,y;
#endif
{
	return (double) powf((float) x, (float) y);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
