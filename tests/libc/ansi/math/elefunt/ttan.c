/* -*-C-*- ttan.c */

#include "elefunt.h"

/*#     program to test tan/cotan
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
#                 be deleted provided the following three
#                 parameters are assigned the values indicated
#
#                 ibeta  - the radix of the floating-point system
#                 it     - the number of base-ibeta digits in the
#                          significand of a floating-point number
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
#         abs, alog, amax1, cotan, float, tan, sqrt
#
#
#     latest revision - december 6, 1979
#
#     author - w. j. cody
#              argonne national laboratory
#
# */

void
ttan()
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
        c1,
        c2,
        del,
        eps,
        epsneg,
        half,
        pi,
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
    half = 0.5e0L;
    ait = (float) it;
    pi = 3.14159265e0L;
    a = ZERO;
    b = pi * 0.25e0L;
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
	    y = x * half;
	    x = y + y;
	    if (j == 4)
	    {
		z = cotan(x);
		zz = cotan(y);
		w = 1.0e0L;
		if (z != ZERO)
		{
		    w = ((half - zz) + half) * ((half + zz) + half);
		    w = (z + w / (zz + zz)) / z;
		}
	    }
	    else
	    {
		z = tan(x);
		zz = tan(y);
		w = 1.0e0L;
		if (z != ZERO)
		{
		    w = ((half - zz) + half) * ((half + zz) + half);
		    w = (z - (zz + zz) / w) / z;
		}
	    }
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
	if (j != 4)
	    printf("\fTEST OF TAN(X) VS 2*TAN(X/2)/(1-TAN(X/2)**2)\n\n\n");
	if (j == 4)
	    printf("\fTEST OF COT(X) VS (COT(X/2)**2-1)/(2*COT(X/2))\n\n\n");
	printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n", n);
	printf("    (" F15P4E "," F15P4E ")\n\n\n", a, b);
	if (j != 4)
	{
	    printf(" TAN(X) WAS LARGER%6d TIMES,\n", k1);
	    printf("            AGREED%6d TIMES, AND\n", k2);
	    printf("       WAS SMALLER%6d TIMES.\n\n", k3);
	}
	if (j == 4)
	{
	    printf(" COT(X) WAS LARGER%6d TIMES,\n", k1);
	    printf("            AGREED%6d TIMES, AND\n", k2);
	    printf("       WAS SMALLER%6d TIMES.\n\n\n", k3);
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
	if (j != 1)
	{
	    a = pi * 6.0e0L;
	    b = a + pi * 0.25e0L;
	}
	else
	{
	    a = pi * 0.875e0L;
	    b = pi * 1.125e0L;
	}
    }

    /* special tests */

    printf("\fSPECIAL TESTS\n\n\n");
    printf(" THE IDENTITY  TAN(-X) = -TAN(X)  WILL BE TESTED.\n\n");
    printf("        X         F(X) + F(-X)\n\n");

    for (i = 1; i <= 5; ++i)
    {
	x = ran(i1) * a;
	z = tan(x) + tan(-x);
	printf(F15P7E F15P7E "\n\n", x, z);
    }

    printf(" THE IDENTITY TAN(X) = X , X SMALL, WILL BE TESTED.\n\n");
    printf("        X         X - F(X)\n\n");
    betap = ipow(beta, it);
    x = ran(i1) / betap;

    for (i = 1; i <= 5; ++i)
    {
	z = x - tan(x);
	printf(F15P7E F15P7E "\n\n", x, z);
	x = x / beta;
    }

    printf(" TEST OF UNDERFLOW FOR VERY SMALL ARGUMENT.\n\n");
    x = pow((float) beta, (float) (minexp * 0.75e0));
    y = tan(x);
    printf("       TAN(" F13P6E ") =" F13P6E "\n\n", x, y);
    c1 = -225.0e0L;
    c2 = -.950846454195142026e0L;
    x = 11.0e0L;
    y = tan(x);
    w = ((c1 - y) + c2) / (c1 + c2);
    z = ALOG(ABS(w)) / albeta;
    printf(" THE RELATIVE ERROR IN TAN(11) IS" F15P7E " = %4d **" F7P2F " WHERE\n\n",
	w, ibeta, z);
    printf("      TAN(" F13P6E ") =" F13P6E "\n\n", x, y);
    w = AMAX1(ait + z, ZERO);
    printf(
	" THE ESTIMATED LOSS OF BASE%4d SIGNIFICANT DIGITS IS" F7P2F "\n\n\n",
	ibeta, w);

    /* test of error returns */

    printf("\fTEST OF ERROR RETURNS\n\n\n");

    x = pow((float) beta, (float) (it / 2));
    printf(" TAN WILL BE CALLED WITH THE ARGUMENT" F15P4E "\n",x);
    printf(" THIS SHOULD NOT TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    y = tan(x);
    if (errno)
	perror("tan()");
    printf(" TAN RETURNED THE VALUE" F15P4E "\n\n\n\n", y);

    x = betap;
    printf("\nTAN WILL BE CALLED WITH THE ARGUMENT" F15P4E "\n", x);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    y = tan(x);
    if (errno)
	perror("tan()");
    printf(" TAN RETURNED THE VALUE" F15P4E "\n\n\n\n", y);

    printf(" THIS CONCLUDES THE TESTS\n");
}
