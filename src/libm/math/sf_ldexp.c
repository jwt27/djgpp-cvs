/* sf_ldexp.c -- float version of s_ldexp.c.
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

#include "fdlibm.h"
#include <errno.h>
#include <float.h>
#include <libc/ieee.h>

#ifdef __STDC__
	float ldexpf(float value, int expon)
#else
	float ldexpf(value, expon)
	float value; int expon;
#endif
{
	_float_long_union uvalue;
	
	uvalue.f = value;

	if(!finitef(uvalue.l)||value==(float)0.0) return value;
	value = scalbnf(value,expon);
	if(!finitef(uvalue.l)||
	   (value < FLT_MIN && value > -FLT_MIN)) errno = ERANGE;
	return value;
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double ldexp(double value, int expon)
#else
	double ldexp(value, expon)
	double value; int expon;
#endif
{
	return (double) ldexpf((float) value, expon);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
