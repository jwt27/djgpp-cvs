/* -*-C-*- derived from texp.c */

#include "elefunt.h"

/***********************************************************************
#     program to test exp
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
#         abs, aint, alog, AMAX1, exp, float, sqrt
#
#
#     latest revision - december 6, 1979
#
#     author - w. j. cody
#              argonne national laboratory
#
#
***********************************************************************/

void
texp2()
{
    int i,
        ibeta,
        iexp,
        irnd,
        it,
        i1,
        j,
        k1,
        k2,
        k3,
	machep,
        maxexp,
        minexp,
        n,
	negep,
        ngrd;
    float a,
        ait,
	albeta,
        b,
        beta,
        d,
        del,
        eps,
        epsneg,
        r6,
        r7,
	v,
        w,
	x,
        xl,
        xmax,
        xmin,
        xn,
	x1,
	y,
	z,
	zz;

    machar(&ibeta, &it, &irnd, &ngrd, &machep, &negep, &iexp, &minexp,
        &maxexp, &eps, &epsneg, &xmin, &xmax);
    beta = (float) ibeta;
    albeta = ALOG(beta);
    ait = (float) it;
    v = 0.0625e0L;
    a = TWO;
    b = ALOG(a) * 0.5e0L;
    a = -b + v;
    d = ALOG(0.9e0L * xmax);
    n = 2000;
    xn = (float) n;
    i1 = 0;

    /* random argument accuracy tests */
    for (j = 1; j <= 3; ++j)
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

	    /* purify arguments */
	    y = x - v;
	    if (y < ZERO)
		x = y + v;
	    z = exp2(x);
	    zz = exp2(y);
	    if (j == 1)
		z = z * exp2(-v);
	    else
	    {
		if (ibeta != 10)
		    /* z = z *.0625e0L - z * 2.4453321046920570389e-3L; */
		    z = z * exp2(-2.8125L);
		else
		    z = z * 6.0e-2L + z * 5.466789530794296106e-5L;
	    }
	    w = ONE;
	    if (zz != ZERO)
		w = (z - zz) / zz;
	    if (w < ZERO)
		k1 = k1 + 1;
	    if (w > ZERO)
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

	printf("\fTEST OF exp2(X-" F7P4F ") VS exp2(X)/exp2(" F7P4F ")\n\n\n", v, v);
	printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n", n);
	printf("      (" F15P4E "," F15P4E ")\n\n\n", a, b);
	printf(" exp2(X-V) WAS LARGER%6d TIMES,\n", k1);
	printf("              AGREED%6d TIMES, AND\n", k2);
	printf("         WAS SMALLER%6d TIMES.\n\n\n", k3);
	printf(
" THERE ARE %4d BASE %4d SIGNIFICANT DIGITS IN A FLOATING-POINT NUMBER\n\n\n",
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
	if (j == 2)
	{
	    a = -TWO * a;
	    b = TEN * a;
	    if (b < d)
		b = d;
	}
	else
	{
	    v = 45.0e0L / 16.0e0L;
	    a = -TEN * b;
	    b = 4.0e0L * xmin * ipow((float) beta, it);
	    b = ALOG(b);
	}
    }

    /* special tests */
    printf("\fSPECIAL TESTS\n\n\n");
    printf(" THE IDENTITY exp2(X)*exp2(-X) = 1.0  WILL BE TESTED.\n\n");
    printf("       X        F(X)*F(-X) - 1\n\n");

    for (i = 1; i <= 5; ++i)
    {
	x = ran(i1) * beta;
	y = -x;
	z = exp2(x) * exp2(y) - ONE;
	printf(F15P7E F15P7E "\n\n", x, z);
    }

    printf("\n\n TEST OF SPECIAL ARGUMENTS\n\n\n");
    x = ZERO;
    y = exp2(x) - ONE;
    printf(" exp2(0.0) - 1.0E0 = " F15P7E "\n\n", y);
    x = AINT(ALOG(xmin));
    y = exp2(x);
    printf(" exp2(" F13P6E ") =" F13P6E "\n\n", x, y);
    x = AINT(ALOG(xmax));
    y = exp2(x);
    printf(" exp2(" F13P6E ") =" F13P6E "\n\n", x, y);
    x = x / TWO;
    v = x / TWO;
    y = exp2(x);
    z = exp2(v);
    z = z * z;
    printf(" IF exp2(" F13P6E ") = " F13P6E " IS NOT ABOUT\n", x, y);
    printf(" exp2(" F13P6E ")**2 =" F13P6E " THERE IS AN ARG RED ERROR\n", v, z);

    /* test of error returns */

    printf("\fTEST OF ERROR RETURNS\n\n\n");
    x = -ONE / sqrt(xmin);
    printf(" exp2 WILL BE CALLED WITH THE ARGUMENT" F15P4E "\n",x);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    y = exp2(x);
    if (errno)
	perror("exp()");
    printf(" exp2 RETURNED THE VALUE" F13P6E "\n\n\n\n", y);
    x = -x;
    printf(" exp2 WILL BE CALLED WITH THE ARGUMENT" F13P6E "\n", x);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    y = exp2(x);
    if (errno)
	perror("exp()");
    printf(" exp2 RETURNED THE VALUE" F13P6E "\n\n\n\n", y);
    printf(" THIS CONCLUDES THE TESTS\n");
}
