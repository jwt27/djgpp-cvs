/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/*
 * cabsf() wrapper for hypotf().
 * 
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include "fdlibm.h"

#ifdef __STDC__
float cabsf(__complex__ float);
float cabsf(__complex__ float z)
#else
float cabsf(z)
    __complex__ float z;
#endif
{
	return hypotf(__real__ z, __imag__ z);
}
