/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/*
 * nan () returns a nan.
 * Added by Cygnus Support.
 */

/*
FUNCTION
	<<nan>>, <<nanf>>---representation of infinity

INDEX
	nan
INDEX
	nanf

ANSI_SYNOPSIS
	#include <math.h>
	double nan(void);
	float nanf(void);

TRAD_SYNOPSIS
	#include <math.h>
	double nan();
	float nanf();


DESCRIPTION
	<<nan>> and <<nanf>> return an IEEE NaN (Not a Number) in
	double and single precision arithmetic respectivly.

QUICKREF
	nan - pure

*/

#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

/* WARNING: Some versions of GCC optimize expressions which
   involve NaNs by emitting constant bit patterns hard-wired
   into the compiler, and ignore the bit pattern in the source
   code.  For example, 7FF80000h below could be ignored and
   FFF80000h used instead.  The use of union initialization
   below was suggested by K.B. Williams and seems to work for
   now, but you better watch future versions of the compiler
   to not mess this up.  */
static const ieee_double_shape_type a_nan = { {0, 0x7ff80000} };

#ifdef __STDC__
	double nan(void)
#else
	double nan()
#endif
{
	return a_nan.value;
}

#endif /* _DOUBLE_IS_32BITS */
