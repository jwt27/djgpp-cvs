/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/*
 * hypotl() function for DJGPP.
 *
 * hypotl() computes sqrt(x^2 + y^2).  The problem with the obvious
 * naive implementation is that it might fail for very large or
 * very small arguments.  For instance, for large x or y the result
 * might overflow even if the value of the function should not,
 * because squaring a large number might trigger an overflow.  For
 * very small numbers, their square might underflow and will be
 * silently replaced by zero; this won't cause an exception, but might
 * have an adverse effect on the accuracy of the result.
 *
 * This implementation tries to avoid the above pitfals, without
 * inflicting too much of a performance hit.
 *
 */
 
#include <float.h>
#include <math.h>
#include <errno.h>

/* Approximate square roots of LDBL_MAX and LDBL_MIN.  Numbers
   between these two shouldn't neither overflow nor underflow
   when squared.  */
#define __SQRT_LDBL_MAX 1.0e+2466
#define __SQRT_LDBL_MIN 1.6e-2466

long double
hypotl(long double x, long double y)
{
  long double abig = x < 0 ? -x : x, asmall = y < 0 ? -y : y;
  long double ratio;
 
  /* Make abig = max(|x|, |y|), asmall = min(|x|, |y|).  */
  if (abig < asmall)
    {
      long double temp = abig;
 
      abig = asmall;
      asmall = temp;
    }
 
  /* Trivial case.  */
  if (asmall == 0.L)
    return abig;
 
  /* Scale the numbers as much as possible by using its ratio.
     For example, if both ABIG and ASMALL are VERY small, then
     X^2 + Y^2 might be VERY inaccurate due to loss of
     significant digits.  Dividing ASMALL by ABIG scales them
     to a certain degree, so that accuracy is better.  */
 
  if ((ratio = asmall / abig) > __SQRT_LDBL_MIN)
    return abig * sqrt(1.0L + ratio*ratio);
  else
    {
      /* Slower but safer algorithm due to Moler and Morrison.  Never
         produces any intermediate result greater than roughly the
         larger of X and Y.  Should converge to machine-precision
         accuracy in 3 iterations.  */
 
      long double r = ratio*ratio, t, s, p = abig, q = asmall;
 
      do {
        t = 4.0L + r;
        if (t == 4.0L)
          break;
        s = r / t;
        p += 2.0L * s * p;
        q *= s;
        r = (q / p) * (q / p);
      } while (1);
 
      return p;
    }
}
 
#ifdef  TEST
 
#include <stdio.h>
 
int
main(void)
{
  printf("hypotl(3, 4) =\t\t\t %25.17Le\n", hypotl(3., 4.));
  printf("hypotl(3*10^150, 4*10^150) =\t %30.21Lg\n",
	 hypotl(3.e+150L, 4.e+150L));
  printf("hypotl(3*10^306, 4*10^306) =\t %30.21Lg\n",
	 hypotl(3.e+306L, 4.e+306L));
  printf("hypotl(3*10^-320, 4*10^-320) =\t %30.21Lg\n",
         hypotl(3.e-320L, 4.e-320L));
  printf("hypotl(0.7*DBL_MAX, 0.7*DBL_MAX) =%30.21Lg\n",
         hypotl(0.7L*DBL_MAX, 0.7L*DBL_MAX));
  printf("hypotl(DBL_MAX, 1.0) =\t\t %30.21Lg\n", hypotl(DBL_MAX, 1.0L));
  printf("hypotl(1.0, DBL_MAX) =\t\t %30.21Lg\n", hypotl(1.0L, DBL_MAX));
  printf("hypotl(0.0, DBL_MAX) =\t\t %30.21Lg\n", hypotl(0.0L, DBL_MAX));
  printf("hypotl(3*10^2463, 4*10^2463) =\t %30.21Lg\n",
	 hypotl(3.e+2463L, 4.e+2463L));
  printf("hypotl(3*10^2467, 4*10^2467) =\t %30.21Lg\n",
	 hypotl(3.e+2467L, 4.e+2467L));
  printf("hypotl(3*10^4930, 4*10^4930) =\t %30.21Lg\n",
	 hypotl(3.e+4930L, 4.e+4930L));
  printf("hypotl(3*10^-4930, 4*10^-4930) =\t %30.21Lg\n",
         hypotl(3.e-4930L, 4.e-4930L));
  printf("hypotl(0.7*LDBL_MAX, 0.7*LDBL_MAX) =%30.21Lg\n",
         hypotl(0.7L*LDBL_MAX, 0.7L*LDBL_MAX));
  printf("hypotl(LDBL_MAX, 1.0) =\t\t %30.21Lg\n", hypotl(LDBL_MAX, 1.0L));
  printf("hypotl(1.0, LDBL_MAX) =\t\t %30.21Lg\n", hypotl(1.0L, LDBL_MAX));
  printf("hypotl(0.0, LDBL_MAX) =\t\t %30.21Lg\n", hypotl(0.0L, LDBL_MAX));
 
  return 0;
}
 
#endif
