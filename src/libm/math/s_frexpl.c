/* s_frexpl.c -- long double version of s_frexp.c.
 * Conversion to 80-bit long double by Juan Manuel Guerrero, juan.guerrero@gmx.de.
 */


/* @(#)s_frexp.c 5.1 93/09/24 */
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
 * for non-zero x 
 *	x = frexp(arg,&exp);
 * return a long double fp quantity x such that 0.5 <= |x| <1.0
 * and the corresponding binary exponent "exp". That is
 *	arg = x*2^exp.
 * If arg is inf, 0.0, or NaN, then frexp(arg,&exp) returns arg 
 * with *exp=0. 
 */

#define IS_INF_OR_NAN(v)       ((v).ldt.exponent == 0x7FFF)
#define IS_DENORMAL(v)         ((v).ldt.exponent == 0x0000 && (((v).ldt.mantissah & 0x80000000) == 0x00000000))
#define IS_PSEUDO_DENORMAL(v)  ((v).ldt.exponent == 0x0000 && (((v).ldt.mantissah & 0x80000000) == 0x80000000))
#define IS_ZERO(v)             (((v).ldt.mantissah | (v).ldt.mantissal) == 0x00000000)

#include <libc/ieee.h>
#include "fdlibm.h"

#ifdef __STDC__
static const long double
#else
static long double
#endif
two65 =  3.6893488147419103232E+19,  /* 0, 0x3FFF + 0x0041, 0x80000000U, 0x00000000U */
two64 =  1.8446744073709551616E+19;  /* 0, 0x3FFF + 0x0040, 0x80000000U, 0x00000000U */
#ifdef __STDC__
	long double frexpl(long double x, int *eptr)
#else
	long double frexpl(x, eptr)
	long double x; int *eptr;
#endif
{
        _longdouble_union_t value;
        value.ld = x;
	*eptr = 0;
	if (IS_INF_OR_NAN(value) || IS_ZERO(value)) return x;	/* 0,inf,nan */
	if (IS_DENORMAL(value)) {		/* subnormal */
	    value.ld *= two65;
	    *eptr = -65;
	}
	else if (IS_PSEUDO_DENORMAL(value)) {	/* pseudo subnormal */
	    value.ld *= two64;
	    *eptr = -64;
	}
	*eptr += value.ldt.exponent - 16382;
	value.ldt.exponent = 16382;
	return value.ld;
}
