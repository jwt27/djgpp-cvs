/* wf_lgamma.c -- float version of w_lgamma.c.
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

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float lgammaf(float x)
#else
	float lgammaf(x)
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_lgammaf_r(x,&signgam);
#else
	_float_long_union ux;
	_float_long_union uy;
    float y;
	
	ux.f = x;

    y = __ieee754_lgammaf_r(x,&signgam);
	uy.f = y;
        if(_LIB_VERSION == _IEEE_) return y;
    if(!finitef(uy.l)&&finitef(ux.l)) {
            if(floorf(x)==x&&x<=(float)0.0)
	        /* lgamma pole */
                return (float)__kernel_standard((double)x,(double)x,115);
            else
	        /* lgamma overflow */
                return (float)__kernel_standard((double)x,(double)x,114);
        } else
            return y;
#endif
}             

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double lgamma(double x)
#else
	double lgamma(x)
	double x;
#endif
{
	return (double) lgammaf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
