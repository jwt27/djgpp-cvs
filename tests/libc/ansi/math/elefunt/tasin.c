/* -*-C-*- tasin.c */

#include "elefunt.h"

/*
#     program to test asin/acos
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
#                 ibeta  - the radix of the floating-point system
#                 it     - the number of base-ibeta digits in the
#                          significand of a floating-point number
#                 irnd   - 0 if floating-point addition chops,
#                          1 if floating-point addition rounds
#                 minexp - the largest in magnitude negative
#                          integer such that float(ibeta)**minexp
#                          is a positive floating-point number
#
#        ran(k) - a function subprogram returning random real
#                 numbers uniformly distributed over (0,1)
#
#
#     standard fortran subprograms required
#
#         abs, acos, alog, alog10, amax1, asin, float, int, sqrt
#
#
#     latest revision - december 6, 1979
#
#     author - w. j. cody
#              argonne national laboratory
#
#*/

/* #undef ABS
#define ABS fabs */


void
tasin()
{
    int i,
        ibeta,
        iexp,
	irnd,
        it,
	i1,
        j,
        k,
        k1,
        k2,
        k3,
        l,
        m,
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
        c1,
        c2,
        del,
        eps,
        epsneg,
        half,
        r6,
        r7,
        s,
        sum,
        w,
        x,
        xl,
        xm,
	xmax,
        xmin,
	xn,
        x1,
        y,
        ysq,
        z,
        zz;

    machar(&ibeta, &it, &irnd, &ngrd, &machep, &negep, &iexp, &minexp,
	&maxexp, &eps, &epsneg, &xmin, &xmax);
    beta = (float) ibeta;
    albeta = ALOG(beta);
    half = 0.5e0L;
    ait = (float) it;
    k = (int) (ALOG10(ipow(beta, it))) + 1;
    if (ibeta != 10)
    {
	c1 = 201.0e0L / 128.0e0L;
	c2 = 4.8382679489661923132e-4L;
    }
    else
    {
	c1 = 1.57e0L;
	c2 = 7.9632679489661923132e-4L;
    }
    a = -0.125e0;
    b = -a;
    n = 2000;
    xn = (float) n;
    i1 = 0;
    l = -1;

    /* random argument accuracy tests */

    for (j = 1; j <= 5; ++j)
    {
	k1 = 0;
	k3 = 0;
	l = -l;
	x1 = ZERO;
	r6 = ZERO;
	r7 = ZERO;
	del = (b - a) / xn;
	xl = a;

	for (i = 1; i <= n; ++i)
	{
	    x = del * ran(i1) + xl;
	    if (j <= 2)
	    {
		y = x;
		ysq = y * y;
	    }
	    else
	    {
		ysq = half - half * ABS(x);
		x = (half - (ysq + ysq)) + half;
		if (j == 5)
		    x = -x;
		y = sqrt(ysq);
		y = y + y;
	    }
	    sum = ZERO;
	    xm = (float) (k + k + 1);
	    if (l > 0)
		z = /* (double) --buers */ asin(x);
	    if (l < 0)
		z = /* (double) --buers */ acos(x);

	    for (m = 1; m <= k; ++m)
	    {
		sum = ysq * (sum + 1.0e0L / xm);
		xm = xm - 2.0e0L;
		sum = sum * (xm / (xm + 1.0e0L));
		/* xm -= 2.0L;
		sum *= xm / (xm + 1.0L); */
	    }

	    sum = sum * y;
	    /* sum *= y; */
	    if (!((j != 1) && (j != 4)))
	    {
		zz = y + sum;
		sum = (y - zz) + sum;
		/* sum += (y - zz);      */
		if (irnd != 1)
		    zz = zz + (sum + sum);
		    /* zz += sum + sum;       */
	    }
	    else
	    {
		s = c1 + c2;
		sum = ((c1 - s) + c2) - sum;
		zz = s + sum;
		sum = ((s - zz) + sum) - y;
		s = zz;
		zz = s + sum;
		sum = (s - zz) + sum;
		if (irnd != 1)
		    zz = zz + (sum + sum);
	    }
	    w = 1.0e0L;
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
	if (l < 0)
	{
	    printf("\fTEST OF ACOS(X) VS TAYLOR SERIES\n\n");
	    printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n", n);
	    printf("      (" F15P4E "," F15P4E ")\n\n\n", a, b);
	    printf(" ACOS(X) WAS LARGER%6d TIMES,\n", k1);
	    printf("             AGREED%6d TIMES, AND\n", k2);
	    printf("        WAS SMALLER%6d TIMES.\n\n", k3);
	}
	else
	{
	    printf("\fTEST OF ASIN(X) VS TAYLOR SERIES\n\n");
	    printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n", n);
	    printf("      (" F15P4E "," F15P4E ")\n\n\n", a, b);
	    printf(" ASIN(X) WAS LARGER%6d TIMES,\n", k1);
	    printf("             AGREED%6d TIMES, AND\n", k2);
	    printf("        WAS SMALLER%6d TIMES.\n\n", k3);
	}
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
	if (j == 2)
	{
	    a = 0.75e0L;
	    b = 1.0e0L;
	}
	if (j == 4)
	{
	    b = -a;
	    a = -1.0e0L;
	    c1 = c1 + c1;
	    c2 = c2 + c2;
	    l = -l;
	}
    }

    /* special tests */

    printf("\fSPECIAL TESTS\n\n");
    printf(" THE IDENTITY  ASIN(-X) = -ASIN(X)  WILL BE TESTED.\n\n");
    printf("        X         F(X) + F(-X)\n");

    for (i = 1; i <= 5; ++i)
    {
	x = ran(i1) * a;
	z = asin(x) + asin(-x);
	printf(F15P7E F15P7E "\n\n", x, z);
    }

    printf(" THE IDENTITY ASIN(X) = X , X SMALL, WILL BE TESTED.\n\n");
    printf("        X         X - F(X)\n");
    betap = ipow(beta, it);
    x = ran(i1) / betap;

    for (i = 1; i <= 5; ++i)
    {
	z = x - asin(x);
	printf(F15P7E F15P7E "\n\n", x, z);
	x = x / beta;
    }

    printf(" TEST OF UNDERFLOW FOR VERY SMALL ARGUMENT.\n\n");
    x = pow(beta, (float) minexp * 0.75e0L);
    y = asin(x);
    printf("       ASIN(" F13P6E ") =" F13P6E "\n\n", x, y);

    /* test of error returns */

    printf("\fTEST OF ERROR RETURNS\n\n\n");

    x = 1.2e0L;
    printf(" ASIN WILL BE CALLED WITH THE ARGUMENT" F15P4E "\n",x);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    y = asin(x);
    if (errno)
	perror("asin()");
    printf(" ASIN RETURNED THE VALUE" F15P4E "\n\n\n", y);

    printf(" THIS CONCLUDES THE TESTS\n");
}
