/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/*
 * cabs() wrapper for hypot().
 * 
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include "fdlibm.h"

struct complex {
	double x;
	double y;
};

#ifdef __STDC__
double cabs(struct complex);
double cabs(struct complex z)
#else
double cabs(z)
     struct complex z;
#endif
{
	return hypot(z.x, z.y);
}
