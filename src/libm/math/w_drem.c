/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/*
 * drem() wrapper for remainder().
 * 
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include "fdlibm.h"

double
drem(x, y)
	double x, y;
{
	return remainder(x, y);
}
