/* -*-C-*- tatan.c */

#include "elefunt.h"

/*
#     program to test atan, atan2
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
#                 be deleted provided the following six
#                 parameters are assigned the values indicated
#
#                 ibeta  - the radix of the floating-point system
#                 it     - the number of base-ibeta digits in the
#                          significand of a floating-point number
#                 irnd   - 0 if floating-point addition chops,
#                          1 if floating-point addition rounds
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
#     standard fortran subprograms required
#
#         abs, alog, amax1, atan, atan2, float, sqrt
#
#
#     latest revision - december 6, 1979
#
#     author - w. j. cody
#              argonne national laboratory
#
#*/

void
tatan()
{
    int i,
        k,
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
        del,
        em,
        eps,
        epsneg,
        expon,
        half,
        ob32,
        r6,
        r7,
        sum,
        w,
        x,
        xl,
	xmax,
        xmin,
        xn,
        xsq,
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
    a = -0.0625e0L;
    b = -a;
    ob32 = b * half;
    n = 2000;
    xn = (float) n;
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
	    if (j == 2)
		x = ((1.0e0L + x * a) - ONE) * 16.0e0L;
	    z = atan(x);
	    if (j == 1)
	    {
		xsq = x * x;
		em = 17.0e0L;
		sum = xsq / em;

		for (k = 1; k <= 7; ++k)
		{
		    em = em - TWO;
		    sum = (ONE / em - sum) * xsq;
		}

		sum = -x * sum;
		zz = x + sum;
		sum = (x - zz) + sum;
		if (irnd == 0)
		    zz = zz + (sum + sum);
	    }
	    else
	    if (j != 2)
	    {
		z = z + z;
		y = x / ((half + x * half) * ((half - x) + half));
		zz = atan(y);
	    }
	    else
	    {
		y = x -.0625e0L;
		y = y / (ONE + x * a);
		zz = (atan(y) - 8.1190004042651526021e-5L) + ob32;
		zz = zz + ob32;
	    }
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
	if (j == 1)
	    printf("\fTEST OF ATAN(X) VS TRUNCATED TAYLOR SERIES\n\n");
	else if (j == 2)
	    printf(
	    "\fTEST OF ATAN(X) VS ATAN(1/16) + ATAN((X-1/16)/(1+X/16))\n\n");
	else if (j > 2)
	    printf("\fTEST OF 2*ATAN(X) VS ATAN(2X/(1-X*X))\n\n");
	printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n",n);
	printf("      (" F15P4E "," F15P4E ")\n\n\n", a, b);
	printf(" ATAN(X) WAS LARGER%6d TIMES,\n", k1);
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
	a = b;
	if (j == 1)
	    b = TWO - sqrt(3.0e0L);
	else if (j == 2)
	    b = sqrt(TWO) - ONE;
	else if (j == 3)
	    b = ONE;
    }

    /* special tests */

    printf("\fSPECIAL TESTS\n\n");
    printf(" THE IDENTITY   ATAN(-X) = -ATAN(X)   WILL BE TESTED.\n\n\n");
    printf("        X         F(X) + F(-X)\n\n");
    a = 5.0e0L;

    for (i = 1; i <= 5; ++i)
    {
	x = ran(i1) * a;
	z = atan(x) + atan(-x);
	printf(F15P7E F15P7E "\n\n", x, z);
    }

    printf(" THE IDENTITY ATAN(X) = X , X SMALL, WILL BE TESTED.\n\n\n");
    printf("        X         X - F(X)\n\n");
    betap = ipow(beta, it);
    x = ran(i1) / betap;

    for (i = 1; i <= 5; ++i)
    {
	z = x - atan(x);
	printf(F15P7E F15P7E "\n\n", x, z);
	x = x / beta;
    }

    printf(" THE IDENTITY ATAN(X/Y) = ATAN2(X,Y) WILL BE TESTED\n");
    printf(" THE FIRST COLUMN OF RESULTS SHOULD BE 0, THE SECOND +-PI\n\n");
    printf("        X             Y     F1(X/Y)-F2(X,Y)F1(X/Y)-F2(X/(-Y))\n");
    a = -TWO;
    b = 4.0e0L;

    for (i = 1; i <= 5; ++i)
    {
	x = ran(i1) * b + a;
	y = ran(i1);
	w = -y;
	z = atan(x / y) - atan2(x, y);
	zz = atan(x / w) - atan2(x, w);
	printf(F15P7E F15P7E F15P7E F15P7E "\n\n", x, y, z, zz);
    }

    printf(" TEST OF UNDERFLOW FOR VERY SMALL ARGUMENT.\n\n");
    expon = (float) minexp *0.75e0L;
    x = pow(beta, expon);
    y = atan(x);
    printf("       ATAN(" F13P6E ") =" F13P6E "\n", x, y);

    /* test of error returns */

    printf("\fTEST OF ERROR RETURNS\n\n\n");

    printf(" ATAN WILL BE CALLED WITH THE ARGUMENT" F15P7E "\n", xmax);
    printf(" THIS SHOULD NOT TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    z = atan(xmax);
    if (errno)
	perror("atan()");
    printf("       ATAN(" F13P6E ") =" F13P6E "\n", xmax, z);

    x = ONE;
    y = ZERO;
    printf(" ATAN2 WILL BE CALLED WITH THE ARGUMENTS\n" F15P7E F15P7E "\n\n", x, y);
    printf(" THIS SHOULD NOT TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    z = atan2(x, y);
    if (errno)
	perror("atan2()");
    printf("       ATAN2(" F13P6E F13P6E ") =" F13P6E "\n", x, y, z);

    printf(" ATAN2 WILL BE CALLED WITH THE ARGUMENTS\n" F15P7E F15P7E "\n\n",
	xmin, xmax);
    printf(" THIS SHOULD NOT TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    z = atan2(xmin, xmax);
    if (errno)
	perror("atan2()");
    printf("       ATAN2(" F13P6E F13P6E ") =" F13P6E "\n", xmin, xmax, z);

    printf(" ATAN2 WILL BE CALLED WITH THE ARGUMENTS\n" F15P7E F15P7E "\n\n",
	xmax, xmin);
    printf(" THIS SHOULD NOT TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    z = atan2(xmax, xmin);
    if (errno)
	perror("atan2()");
    printf("       ATAN2(" F13P6E F13P6E ") =" F13P6E "\n", xmax, xmin, z);

    x = ZERO;
    printf(" ATAN2 WILL BE CALLED WITH THE ARGUMENTS\n" F15P7E F15P7E "\n\n", x, y);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    z = atan2(x, y);
    if (errno)
	perror("atan2()");
    printf("       ATAN2(" F13P6E F13P6E ") =" F13P6E "\n", x, y, z);

    printf(" THIS CONCLUDES THE TESTS\n");
}
