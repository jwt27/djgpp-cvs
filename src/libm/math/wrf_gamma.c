/* wrf_gamma.c -- float version of wr_gamma.c.
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
 * wrapper float gammaf_r(float x, int *signgamp)
 */

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
	float gammaf_r(float x, int *signgamp) /* wrapper lgammaf_r */
#else
	float gammaf_r(x,signgamp)              /* wrapper lgammaf_r */
	float x; int *signgamp;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_gammaf_r(x,signgamp);
#else
	_float_long_union ux;
	_float_long_union uy;
	float y;
	
	ux.f = x;

        y = __ieee754_gammaf_r(x,signgamp);
		uy.f = y;
        if(_LIB_VERSION == _IEEE_) return y;
        if(!finitef(uy.f)&&finitef(ux.f)) {
            if(floorf(x)==x&&x<=(float)0.0)
	        /* gammaf pole */
                return (float)__kernel_standard((double)x,(double)x,141);
            else
	        /* gamma overflow */
                return (float)__kernel_standard((double)x,(double)x,140);
        } else
            return y;
#endif
}             
