/* -*-C-*- talog.c */

#include "elefunt.h"

/*#     program to test alog
#
#     data required
#
#        none
#
#     subprograms required from this package
#
#        machar - an environmental inquiry program providing
#                 information on the floating-point arithmetic
#                 system.  note that the call to machar can
#                 be deleted provided the following four
#                 parameters are assigned the values indicated
#
#                 ibeta - the radix of the floating-point system
#                 it    - the number of base-ibeta digits in the
#                         significand of a floating-point number
#                 xmin  - the smallest non-vanishing floating-point
#                         power of the radix
#                 xmax  - the largest finite floating-point no.
#
#        ran(k) - a function subprogram returning random real
#                 numbers uniformly distributed over (0,1)
#
#
#     standard fortran subprograms required
#
#         abs, alog, alog10, amax1, float, sign, sqrt
#
#
#     latest revision - december 6, 1979
#
#     author - w. j. cody
#              argonne national laboratory
#
#*/

void
talog()
{
    int             i, ibeta, iexp, irnd, it, i1, j, k1, k2, k3,
		    machep, maxexp, minexp, n, negep, ngrd;
    float           a, ait, albeta, b, beta, c, del, eight, eps,
		    epsneg, half, r6, r7, tenth, w, x, xl,
		    xmax, xmin, xn, x1, y, z, zz;

    machar(&ibeta, &it, &irnd, &ngrd, &machep, &negep, &iexp, &minexp,
	   &maxexp, &eps, &epsneg, &xmin, &xmax);

    beta = (float) ibeta;
    albeta = ALOG(beta);
    ait = (float) it;
    j = it / 3;
    half = 0.5e0L;
    eight = 8.0e0L;
    tenth = 0.1e0L;
    c = ONE;

    for (i = 1; i <= j; ++i)
	c = c / beta;

    b = ONE + c;
    a = ONE - c;
    n = 2000;
    xn = (float) (n);
    i1 = 0;

    /* random argument accuracy tests */

    for (j = 1; j <= 4; ++j)
    {
	k1 = 0;
	k3 = 0;
	x1 = ZERO;
	r6 = ZERO;
	r7 = ZERO;
	del = (b - a) / xn;
	xl = a;

	for (i = 1; i <= n; ++i)
	{
	    x = del * ran(i1) + xl;
	    if (j == 1)
	    {
		y = (x - half);
		y -= half;
		zz = ALOG(x);
		z = ONE / 3.0e0;
		z = y * (z - y / 4.0e0);
		z = (z - half) * y * y + y;
	    }
	    else if (j == 2)
	    {
		x += eight;
		x -= eight;
		y = x + x / 16.0e0;
		z = ALOG(x);
		/* zz = ALOG(y) - 7.7746816434842581e-5;
		zz = zz - 31.0e0 / 512.0e0; */
		zz = ALOG(y) - ALOG(17.0L/16.0L);
	    }
	    else if (j != 3)
	    {
		z = ALOG(x * x);
		zz = ALOG(x);
		zz = zz + zz;
	    }
	    else
	    {
		x += eight;
		x -= eight;
		y = x + x * tenth;
		/* y = 1.1L*x; */
		z = ALOG10(x);
		zz = ALOG10(y) - 3.7706015822504075e-4L;
		zz = zz - 21.0e0L / 512.0e0L;
		/* zz = ALOG10(y) - ALOG10(1.1L); */
	    }
	    w = ONE;
	    if (z != ZERO)
		w = (z - zz) / z;
	    z = SIGN(w, z);
	    if (z > ZERO)
		k1 = k1 + 1;
	    if (z < ZERO)
		k3 = k3 + 1;
	    w = ABS(w);
	    if (w > r6)
	    {
		r6 = w;
		x1 = x;
	    }
	    r7 = r7 + w * w;
	    xl = xl + del;
	}

	k2 = n - k3 - k1;
	r7 = sqrt(r7 / xn);
	if (j == 1)
	    printf("\fTEST OF ALOG(X) VS T.S. EXPANSION OF ALOG(1+Y)\n\n\n");
	else if (j == 2)
	    printf("\fTEST OF ALOG(X) VS ALOG(17X/16)-ALOG(17/16)\n\n\n");
	else if (j == 3)
	    printf("\fTEST OF ALOG10(X) VS ALOG10(11X/10)-ALOG10(11/10)\n\n\n");
	else if (j == 4)
	    printf("\fTEST OF ALOG(X*X) VS 2 * LOG(X)\n\n\n");
	if (j == 1)
	{
	    printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n", n);
	    printf("      (1-EPS,1+EPS), WHERE EPS =" F15P4E "\n\n\n", c);
	}
	else
	{
	    printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n", n);
	    printf("      (" F15P4E "," F15P4E ")\n\n\n", a, b);
	}
	if (j != 3)
	{
	    printf(" ALOG(X) WAS LARGER%6d TIMES,\n", k1);
	    printf("             AGREED%6d TIMES, AND\n", k2);
	    printf("        WAS SMALLER%6d TIMES.\n\n\n", k3);
	}
	else
	{
	    printf(" ALOG10(X) WAS LARGER%6d TIMES,\n", k1);
	    printf("               AGREED%6d TIMES, AND\n", k2);
	    printf("          WAS SMALLER%6d TIMES.\n\n\n", k3);
	}
	printf(
		  " THERE ARE%4d BASE%4d SIGNIFICANT DIGITS IN A FLOATING-POINT NUMBER\n\n\n",
		  it, ibeta);
	w = -999.0e0;
	if (r6 != ZERO)
	    w = ALOG(ABS(r6)) / albeta;
	printf(" THE MAXIMUM RELATIVE ERROR OF" F15P4E " = %4d **" F7P2F "\n",
	       r6, ibeta, w);
	printf("    OCCURRED FOR X =" F17P6E "\n", x1);
	w = AMAX1(ait + w, ZERO);
	printf(
		  " THE ESTIMATED LOSS OF BASE%4d SIGNIFICANT DIGITS IS" F7P2F "\n\n\n",
		  ibeta, w);
	w = -999.0e0;
	if (r7 != ZERO)
	    w = ALOG(ABS(r7)) / albeta;
	printf(" THE ROOT MEAN SQUARE RELATIVE ERROR WAS" F15P4E " = %4d **" F7P2F "\n",
	       r7, ibeta, w);
	w = AMAX1(ait + w, ZERO);
	printf(
		  " THE ESTIMATED LOSS OF BASE%4d SIGNIFICANT DIGITS IS" F7P2F "\n\n\n",
		  ibeta, w);
	if (j <= 1)
	{
	    a = sqrt(half);
	    b = 15.0e0 / 16.0e0;
	}
	else if (j > 2)
	{
	    a = 16.0e0;
	    b = 240.0e0;
	}
	else
	{
	    a = sqrt(tenth);
	    b = 0.9e0;
	}
    }

    /* special tests */

    printf("\fSPECIAL TESTS\n\n\n");
    printf(" THE IDENTITY  ALOG(X) = -ALOG(1/X)  WILL BE TESTED.\n\n");
    printf("        X         F(X) + F(1/X)\n\n");

    for (i = 1; i <= 5; ++i)
    {
	x = ran(i1);
	x = x + x + 15.0e0;
	y = ONE / x;
	z = ALOG(x) + ALOG(y);
	printf(F15P7E F15P7E "\n\n", x, z);
    }

    printf("\n\n TEST OF SPECIAL ARGUMENTS\n\n\n");
    x = ONE;
    y = ALOG(x);
    printf(" ALOG(1.0) = " F15P7E "\n\n\n", y);
    x = xmin;
    y = ALOG(x);
    printf(" ALOG(XMIN) = ALOG(" F15P7E ") = " F15P7E "\n\n\n", x, y);
    x = xmax;
    y = ALOG(x);
    printf(" ALOG(XMAX) = ALOG(" F15P7E ") = " F15P7E "\n\n\n", x, y);

    /* test of error returns */

    printf("\fTEST OF ERROR RETURNS\n\n\n");

    x = -2.0e0;
    printf(" ALOG WILL BE CALLED WITH THE ARGUMENT" F15P4E "\n", x);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    y = ALOG(x);
    if (errno)
	perror("ALOG()");
    printf(" ALOG RETURNED THE VALUE" F15P4E "\n\n\n", y);

    x = ZERO;
    printf(" ALOG WILL BE CALLED WITH THE ARGUMENT" F15P4E "\n", x);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    y = ALOG(x);
    if (errno)
	perror("ALOG()");
    printf(" ALOG RETURNED THE VALUE" F15P4E "\n\n\n\n", y);

    printf(" THIS CONCLUDES THE TESTS\n");
}
