/* ef_hypot.c -- float version of e_hypot.c.
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

#define	SQRT_FLT_MAX	1.84467429742e+19  /* 0x5f7fffff */

#include <float.h>
#include "fdlibm.h"

#ifdef __STDC__
	float __ieee754_hypotf(float x, float y)
#else
	float __ieee754_hypotf(x,y)
	float x, y;
#endif
{
	float a=x,b=y,t1,t2,w;
	__int32_t j,ha,hb;

	if (isnanf(x) || isnanf(y))
		return (x-x)/(y-y);
	if (isinff(x) || isinff(y))
		return __kernel_standard(x, y, 104);
	GET_FLOAT_WORD(ha,x);
	ha &= 0x7fffffff;
	GET_FLOAT_WORD(hb,y);
	hb &= 0x7fffffff;
	if(hb < ha) {a=y;b=x;j=ha; ha=hb;hb=j;} else {a=x;b=y;}
	SET_FLOAT_WORD(a,ha);	/* a <- |a| */
	SET_FLOAT_WORD(b,hb);	/* b <- |b| */
	t1 = (a > 0) ? (a/b) : 0;
	if ((t1 >= SQRT_FLT_MAX) && finitef(x) && finitef(y))
		return __kernel_standard(x, y, 104);
	t1 *= t1;
	t2 = sqrtf(++t1);
	if ((t2 > 1) && (b >= FLT_MAX))
		return __kernel_standard(x, y, 104);
	w = t2 * b;

	       return w;
}
