/*
 * cabsf() wrapper for hypotf().
 * 
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include "fdlibm.h"

struct complex {
	float x;
	float y;
};

#ifdef __STDC__
float cabsf(struct complex);
float cabsf(struct complex z)
#else
float cabsf(z)
	struct complex z;
#endif
{
	return hypotf(z.x, z.y);
}
