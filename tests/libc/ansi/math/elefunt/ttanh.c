/* -*-C-*- ttanh.c */

#include "elefunt.h"

/*
#     program to test tanh
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
#                 be deleted provided the following five
#                 parameters are assigned the values indicated
#
#                 ibeta  - the radix of the floating-point system
#                 it     - the number of base-ibeta digits in the
#                          significand of a floating-point number
#                 minexp - the largest in magnitude negative
#                          integer such that float(ibeta)**minexp
#                          is a positive floating-point number
#                 xmin   - the smallest non-vanishing floating-point
#                          power of the radix
#                 xmax   - the largest finite floating-point no.
#
#        ran(k) - a function subprogram returning random real
#                 numbers uniformly distributed over (0,1)
#
#
#     standard fortran subprograms required
#
#         abs, alog, amax1, float, sqrt, tanh
#
#
#     latest revision - december 6, 1979
#
#     author - w. j. cody
#              argonne national laboratory
#
#*/

void
ttanh()
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
        betap,
        c,
        d,
        del,
        eps,
        epsneg,
        expon,
        half,
        r6,
        r7,
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
    half = 0.5e0L;
    a = 0.125e0L;
    b = ALOG(3.0e0L) * half;
    c = 1.2435300177159620805e-1L;
    d = ALOG(2.0e0L) + (ait + ONE) * ALOG(beta) * half;
    n = 2000;
    xn = (float) n;
    i1 = 0;

    /* random argument accuracy tests */

    for (j = 1; j <= 2; ++j)
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
	    z = tanh(x);
	    y = x - 0.125e0L;
	    zz = tanh(y);
	    zz = (zz + c) / (ONE + c * zz);
	    w = ONE;
	    if (z != ZERO)
		w = (z - zz) / z;
	    if (w > ZERO)
		k1 = k1 + 1;
	    if (w < ZERO)
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
	printf(
"\fTEST OF TANH(X) VS (TANH(X-1/8)+TANH(1/8))/(1+TANH(X-1/8)TANH(1/8))\n\n");
	printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n", n);
	printf("      (" F15P4E "," F15P4E ")\n\n\n", a, b);
	printf(" TANH(X) WAS LARGER%6d TIMES,\n", k1);
	printf("             AGREED%6d TIMES, AND\n", k2);
	printf("        WAS SMALLER%6d TIMES.\n\n", k3);
	printf(
" THERE ARE%4d BASE%4d SIGNIFICANT DIGITS IN A FLOATING-POINT NUMBER\n\n",
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
	a = b + a;
	b = d;
    }

    /* special tests */

    printf("\fSPECIAL TESTS\n\n");
    printf(" THE IDENTITY   TANH(-X) = -TANH(X)   WILL BE TESTED.\n\n\n");
    printf("        X         F(X) + F(-X)\n\n");

    for (i = 1; i <= 5; ++i)
    {
	x = ran(i1);
	z = tanh(x) + tanh(-x);
	printf(F15P7E F15P7E "\n\n", x, z);
    }

    printf(" THE IDENTITY TANH(X) = X , X SMALL, WILL BE TESTED.\n\n\n");
    printf("        X         X - F(X)\n\n");
    betap = ipow(beta, it);
    x = ran(i1) / betap;

    for (i = 1; i <= 5; ++i)
    {
	z = x - tanh(x);
	printf(F15P7E F15P7E "\n\n", x, z);
	x = x / beta;
    }

    printf(" THE IDENTITY TANH(X) = 1 , X LARGE, WILL BE TESTED.\n\n\n");
    printf("        X         1 - F(X)\n\n");
    x = d;
    b = 4.0e0L;

    for (i = 1; i <= 5; ++i)
    {
	z = (tanh(x) - half) - half;
	printf(F15P7E F15P7E "\n\n", x, z);
	x = x + b;
    }

    printf(" TEST OF UNDERFLOW FOR VERY SMALL ARGUMENT.\n\n");
    expon = (float) minexp *0.75e0L;
    x = pow(beta, expon);
    z = tanh(x);
    printf("       TANH(" F13P6E ") =" F13P6E "\n", x, z);
    printf(" THE FUNCTION TANH WILL BE CALLED WITH THE ARGUMENT" F15P7E "\n", xmax);
    z = tanh(xmax);
    printf("       TANH(" F13P6E ") =" F13P6E "\n", xmax, z);
    printf(" THE FUNCTION TANH WILL BE CALLED WITH THE ARGUMENT" F15P7E "\n", xmin);
    z = tanh(xmin);
    printf("       TANH(" F13P6E ") =" F13P6E "\n", xmin, z);
    x = ZERO;
    printf(" THE FUNCTION TANH WILL BE CALLED WITH THE ARGUMENT" F15P7E "\n", x);
    z = tanh(x);
    printf("       TANH(" F13P6E ") =" F13P6E "\n", x, z);
    printf(" THIS CONCLUDES THE TESTS\n");
}
