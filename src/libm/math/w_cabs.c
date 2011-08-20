/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/*
 * cabs() wrapper for hypot().
 * 
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include "fdlibm.h"

#ifdef __STDC__
double cabs(_Complex double);
double cabs(_Complex double z)
#else
double cabs(z)
     _Complex double z;
#endif
{
	return hypot(__real__ z, __imag__ z);
}
