/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/*
 * infinityf () returns the representation of infinity.
 * Added by Cygnus Support.
 */

#include "fdlibm.h"

#ifdef __STDC__
	float infinityf(void)
#else
	float infinityf()
#endif
{
	float x;

	SET_FLOAT_WORD(x,0x7f800000);
	return x;
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double infinityf(void)
#else
	double infinity()
#endif
{
	return (double) infinityf();
}

#endif /* defined(_DOUBLE_IS_32BITS) */
