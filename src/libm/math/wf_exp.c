/* wf_exp.c -- float version of w_exp.c.
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
 * wrapper expf(x)
 */

#include "fdlibm.h"
#include <libc/ieee.h>

#ifdef __STDC__
static const float
#else
static float
#endif
o_threshold=  8.872283911e+01,	 /* 0x42b17218 */
u_threshold= -1.0397208405e+02;  /* 0xc2cff1b5 */

#ifdef __STDC__
	float expf(float x)		/* wrapper expf */
#else
	float expf(x)			/* wrapper expf */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_expf(x);
#else
	_float_long_union ux;
	float z;
	
	ux.f = x;

	z = __ieee754_expf(x);
	if(_LIB_VERSION == _IEEE_) return z;
	if(finitef(ux.l)) {
	    if(x>o_threshold)
	        /* exp overflow */
	        return (float)__kernel_standard((double)x,(double)x,106);
	    else if(x<u_threshold)
	        /* exp underflow */
	        return (float)__kernel_standard((double)x,(double)x,107);
	} 
	return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double exp(double x)
#else
	double exp(x)
	double x;
#endif
{
	return (double) expf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
