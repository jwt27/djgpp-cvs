/* -*-C-*- tsinh.c */

#include "elefunt.h"

/*#     program to test sinh/cosh
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
#                 eps    - the smallest positive floating-point
#                          number such that 1.0+eps != 1.0
#                 xmax   - the largest finite floating-point no.
#
#        ran(k) - a function subprogram returning random real
#                 numbers uniformly distributed over (0,1)
#
#
#     standard fortran subprograms required
#
#         abs, alog, amax1, cosh, float, int, sinh, sqrt
#
#
#     latest revision - december 6, 1979
#
#     author - w. j. cody
#              argonne national laboratory
#
#*/

void
tsinh()
{
    int i,
        ibeta,
        iexp,
        ii,
        irnd,
        it,
        i1,
        i2,
        j,
        k1,
        k2,
        k3,
        machep,
        maxexp,
        minexp,
        n,
        negep,
        ngrd,
        nit;
    float a,
        aind,
        ait,
        albeta,
        alxmax,
        b,
        beta,
        betap,
        c,
        c0,
        del,
        den,
        eps,
        epsneg,
        five,
        r6,
        r7,
        three,
        w,
        x,
        xl,
        xmax,
        xmin,
        xn,
        x1,
        xsq,
        y,
        z,
        zz;

    machar(&ibeta, &it, &irnd, &ngrd, &machep, &negep, &iexp, &minexp,
        &maxexp, &eps, &epsneg, &xmin, &xmax);
    beta = (float) ibeta;
    albeta = ALOG(beta);
    alxmax = ALOG(xmax);
    ait = (float) it;
    three = 3.0e0L;
    five = 5.0e0L;
    c0 = five / 16.0e0L + 1.152713683194269979e-2L;
    a = ZERO;
    b = 0.5e0L;
    c = (ait + ONE) * 0.35e0L;
    if (ibeta == 10)
	c = c * three;
    n = 2000;
    xn = (float) n;
    i1 = 0;
    i2 = 2;
    nit = 2 - (INT(ALOG(eps) * three)) / 20;
    aind = (float) nit + nit + 1;

    /* random argument accuracy tests */

    for (j = 1; j <= 4; ++j)
    {
	if (j == 2)
	{
	    aind = aind - ONE;
	    i2 = 1;
	}
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
	    if (j > 2)
	    {
		y = x;
		x = y - ONE;
		w = x - ONE;
		if (j == 4)
		{
		    z = cosh(x);
		    zz = (cosh(y) + cosh(w)) * c0;
		}
		else
		{
		    z = sinh(x);
		    zz = (sinh(y) + sinh(w)) * c0;
		}
	    }
	    else
	    {
		xsq = x * x;
		zz = ONE;
		den = aind;
#
		for (ii = i2; ii <= nit; ++ii)
		{
		    w = zz * xsq / (den * (den - ONE));
		    zz = w + ONE;
		    den = den - 2.0e0L;
		}
#
		if (j == 2)
		{
		    z = cosh(x);
		    if (irnd == 0)
		    {
			w = (ONE - zz) + w;
			zz = zz + (w + w);
		    }
		}
		else
		{
		    w = x * xsq * zz / 6.0e0L;
		    zz = x + w;
		    z = sinh(x);
		    if (irnd == 0)
		    {
			w = (x - zz) + w;
			zz = zz + (w + w);
		    }
		}
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
	i = (j / 2) * 2;
	if (j == 1)
	    printf("\fTEST OF SINH(X) VS T.S. EXPANSION OF SINH(X)\n\n");
	else if (j == 2)
	    printf("\fTEST OF COSH(X) VS T.S. EXPANSION OF COSH(X)\n\n");
	else if (j == 3)
	    printf("\fTEST OF SINH(X) VS C*(SINH(X+1)+SINH(X-1))\n\n");
	else if (j == 4)
	    printf("\fTEST OF COSH(X) VS C*(COSH(X+1)+COSH(X-1))\n\n");
	printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n", n);
	printf("      (" F15P4E "," F15P4E ")\n\n\n", a, b);
	if (i != j)
	{
	    printf(" SINH(X) WAS LARGER%6d TIMES,\n", k1);
	    printf("             AGREED%6d TIMES, AND\n", k2);
	    printf("        WAS SMALLER%6d TIMES.\n\n\n", k3);
	}
	else
	{
	    printf(" COSH(X) WAS LARGER%6d TIMES,\n", k1);
	    printf("             AGREED%6d TIMES, AND\n", k2);
	    printf("        WAS SMALLER%6d TIMES.\n\n\n", k3);
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
	    w = ALOG(ABS(r7)) / albeta;	/* */
	printf(" THE ROOT MEAN SQUARE RELATIVE ERROR WAS" F15P4E " = %4d **" F7P2F "\n",
	    r7, ibeta, w);
	w = AMAX1(ait + w, ZERO);
	printf(
	    " THE ESTIMATED LOSS OF BASE%4d SIGNIFICANT DIGITS IS" F7P2F "\n\n\n",
	    ibeta, w);
	if (j == 2)
	{
	    b = alxmax;
	    a = three;
	}
    }

    /* special tests */

    printf("\fSPECIAL TESTS\n\n");
    printf(" THE IDENTITY  SINH(-X) = -SINH(X)  WILL BE TESTED.\n\n");
    printf("        X            F(X) + F(-X)\n\n");

    for (i = 1; i <= 5; ++i)
    {
	x = ran(i1) * a;
	z = sinh(x) + sinh(-x);
	printf(F15P7E F15P7E "\n\n", x, z);
    }

    printf(" THE IDENTITY SINH(X) = X , X SMALL, WILL BE TESTED.\n\n");
    printf("        X         X - F(X)\n\n");
    betap = ipow(beta, it);
    x = ran(i1) / betap;

    for (i = 1; i <= 5; ++i)
    {
	z = x - sinh(x);
	printf(F15P7E F15P7E "\n\n", x, z);
	x = x / beta;
    }

    printf(" THE IDENTITY  COSH(-X) = COSH(X)  WILL BE TESTED.\n\n");
    printf("        X         F(X) - F(-X)\n\n");

    for (i = 1; i <= 5; ++i)
    {
	x = ran(i1) * a;
	z = cosh(x) - cosh(-x);
	printf(F15P7E F15P7E "\n\n", x, z);
    }

    printf(" TEST OF UNDERFLOW FOR VERY SMALL ARGUMENT.\n\n");
    x = pow((float) beta, (float) (FLOAT(minexp) * 0.75e0));
    y = sinh(x);
    printf("      SINH(" F13P6E ") =" F13P6E "\n\n", x, y);

    /* test of error returns */

    printf("\fTEST OF ERROR RETURNS\n\n\n");

    x = alxmax + 0.125e0L;
    printf(" SINH WILL BE CALLED WITH THE ARGUMENT" F15P4E "\n", x);
    printf(" THIS SHOULD NOT TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    y = sinh(x);
    if (errno)
	perror("sinh()");
    printf(" SINH RETURNED THE VALUE" F15P4E "\n\n\n", y);

    x = betap;
    printf("\nSINH WILL BE CALLED WITH THE ARGUMENT" F15P4E "\n",x);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    y = sinh(x);
    if (errno)
	perror("sinh()");
    printf(" SINH RETURNED THE VALUE" F15P4E "\n\n\n\n", y);

    printf(" THIS CONCLUDES THE TESTS\n\n");
}
