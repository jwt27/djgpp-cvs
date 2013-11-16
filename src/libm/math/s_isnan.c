/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */

/* @(#)s_isnan.c 5.1 93/09/24 */
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
FUNCTION
	<<isnan>>,<<isnanf>>,<<isnanl>>,<<isinf>>,<<isinff>>,<<isinfl>>,<<finite>>,<<finitef>>,<<finitel>>---test for exceptional numbers

INDEX
	isnan
INDEX
	isinf
INDEX
	finite

INDEX
	isnanf
INDEX
	isinff
INDEX
	finitef

INDEX
	isnanl
INDEX
	isinfl
INDEX
	finitel


ANSI_SYNOPSIS
	#include <ieeefp.h>
	int isnan(double <[arg]>);
	int isinf(double <[arg]>);
	int finite(double <[arg]>);
	int isnanf(float <[arg]>);
	int isinff(float <[arg]>);
	int finitef(float <[arg]>);
	int isnanl(long double <[arg]>);
	int isinfl(long double <[arg]>);
	int finitel(long double <[arg]>);

TRAD_SYNOPSIS
	#include <ieeefp.h>
	int isnan(<[arg]>)
	double <[arg]>;
	int isinf(<[arg]>)
	double <[arg]>;
	int finite(<[arg]>);
	double <[arg]>;
	int isnanf(<[arg]>);
	float <[arg]>;
	int isinff(<[arg]>);
	float <[arg]>;
	int finitef(<[arg]>);
	float <[arg]>;
	int isnanl(<[arg]>)
	long double <[arg]>;
	int isinfl(<[arg]>)
	long double <[arg]>;
	int finitel(<[arg]>);
	long double <[arg]>;


DESCRIPTION
	These functions provide information on the floating point
	argument supplied.

	There are five major number formats - 
	o+
	o zero
	 a number which contains all zero bits.
	o subnormal
	 Is used to represent  number with a zero exponent, but a non zero fraction.
	 o normal
	  A number with an exponent, and a fraction
     	o infinity
	  A number with an all 1's exponent and a zero fraction.
	o NAN
	  A number with an all 1's exponent and a non zero fraction.

	o-

	<<isnan>> returns 1 if the argument is a nan. <<isinf>>
	returns 1 if the argument is infinity.  <<finite>> returns 1 if the
	argument is zero, subnormal or normal.
	
	The <<isnanf>>, <<isinff>> and <<finitef>> perform the same
	operations as their <<isnan>>, <<isinf>> and <<finite>>
	counterparts, but on single precision floating point numbers.

	The <<isnanl>>, <<isinfl>> and <<finitel>> perform the same
	operations as their <<isnan>>, <<isinf>> and <<finite>>
	counterparts, but on long double precision floating point numbers.

QUICKREF
	isnan - pure
QUICKREF
	isinf - pure
QUICKREF
	finite - pure
QUICKREF
	isnan - pure
QUICKREF
	isinf - pure
QUICKREF
	finite - pure
*/

/*
 * isnan(x) returns 1 is x is nan, else 0;
 * no branching!
 */

#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	int isnan(double x)
#else
	int isnan(x)
	double x;
#endif
{
	__int32_t hx,lx;
	EXTRACT_WORDS(hx,lx,x);
	hx &= 0x7fffffff;
	hx |= (__uint32_t)(lx|(-lx))>>31;	
	hx = 0x7ff00000 - hx;
	return (int)(((__uint32_t)(hx))>>31);
}

#endif /* _DOUBLE_IS_32BITS */
