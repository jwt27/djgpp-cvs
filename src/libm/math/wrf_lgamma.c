/* wrf_lgamma.c -- float version of wr_lgamma.c.
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
 * wrapper float lgammaf_r(float x, int *signgamp)
 */

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float lgammaf_r(float x, int *signgamp) /* wrapper lgammaf_r */
#else
	float lgammaf_r(x,signgamp)              /* wrapper lgammaf_r */
	float x; int *signgamp;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_lgammaf_r(x,signgamp);
#else
	_float_long_union ux;
	_float_long_union uy;
	float y;
	
	ux.f = x;
	
        y = __ieee754_lgammaf_r(x,signgamp);
	uy.f = y;

        if(_LIB_VERSION == _IEEE_) return y;
    if(!finitef(uy.f)&&finitef(ux.f)) {
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
