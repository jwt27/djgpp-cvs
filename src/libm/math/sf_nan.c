/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/*
 * nanf () returns a nan.
 * Added by Cygnus Support.
 */

#include "fdlibm.h"

/* WARNING: Some versions of GCC optimize expressions which
   involve NaNs by emitting constant bit patterns hard-wired
   into the compiler, and ignore the bit pattern in the source
   code.  For example, 7FC00000h below could be ignored and
   FFC00000h used instead.  The use of union initialization
   below was suggested by K.B. Williams and seems to work for
   now, but you better watch future versions of compiler
   to not mess this up.  */
static const ieee_float_shape_type a_nan = { 0x7fc00000 };

	float nanf()
{
	return a_nan.value;
}

#ifdef _DOUBLE_IS_32BITS

	double nan()
{
	return (double) nanf();
}

#endif /* defined(_DOUBLE_IS_32BITS) */
