/* -*-C-*- tpower.c */

#include "elefunt.h"

/*#     program to test power function (**)
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
#                 minexp - the largest in magnitude negative
#                          integer such that  float(ibeta)**minexp
#                          is a positive floating-point number
#                 maxexp - the largest positive integer exponent
#                          for a finite floating-point number
#                 xmin   - the smallest non-vanishing floating-point
#                          power of the radix
#                 xmax   - the largest finite floating-point
#                          number
#
#        ran(k) - a function subprogram returning random real
#                 numbers uniformly distributed over (0,1)
#
#
#     standard fortran subprograms required
#
#         abs, alog, amax1, exp, float, sqrt
#
#
#     latest revision - december 6, 1979
#
#     author - w. j. cody
#              argonne national laboratory
#
#**********************************************************************/

#if 0
#ifdef __BORLANDC__
int
_matherrl (struct _exceptionl *a)
{
    printf("name %s args %.20Le %.20Le\n", a->name, a->arg1, a->arg2);
    a->retval = 0.0L;
    return 1;
}
#endif
#endif

void
tpower()
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
	l,
	machep,
        maxexp,
        minexp,
        n,
        negep,
        ngrd;
    float a,
        ait,
	albeta,
        alxmax,
        b,
        beta,
	c,
        del,
        dely,
        eps,
        epsneg,
	onep5,
	r6,
	r7,
        scale;
    float w,
        x,
        xl,
        xmax,
        xmin,
	xn,
	xsq,
	x1,
	y,
	y1,
	y2,
	z,
	zz;

#ifdef LDOUBLE
    l = 6;
#else
    l = 4;
#endif
    machar(&ibeta, &it, &irnd, &ngrd, &machep, &negep, &iexp, &minexp,
	&maxexp, &eps, &epsneg, &xmin, &xmax);
    beta = (float) ibeta;
    albeta = ALOG(beta);
    ait = (float) it;
    alxmax = ALOG(xmax);
    onep5 = (TWO + ONE) / TWO;
    scale = ONE;
    j = (it + 1) / 2;
    for (i = 1; i <= j; i++)	/* do i = 1, j */
	scale = scale * beta;
    a = ONE / beta;
    b = ONE;
#ifdef __TURBOC__
    /* c = -AMAX1(alxmax, -ALOG(xmin*1e100L)-ALOG(1e100L)) / ALOG(100e0L); */
    c = -alxmax/ALOG(100e0L);
#else
    c = -AMAX1(alxmax, -ALOG(xmin)) / ALOG(100e0L);
#endif
    dely = -c - c;
    n = 2000;
    xn = (float) n;
    i1 = 0;
    y1 = ZERO;

    /* random argument accuracy tests */

    for (j = 1; j <= l; j++)
    {				/* do j = 1, 4 */
	k1 = 0;
	k3 = 0;
	x1 = ZERO;
	r6 = ZERO;
	r7 = ZERO;
	del = (b - a) / xn;
	xl = a;

	for (i = 1; i <= n; i++)
	{			/* DO I = 1, N */
	    x = del * ran(i1) + xl;
	    if (j == 1)
	    {
		zz = pow(x, ONE);	/* zz=x**ONE */
		z = x;
	    }
	    else
	    {
		w = scale * x;
		x += w;
		x -= w;
		xsq = x * x;
		if (j != 4 && j != 6)
		{
		    zz = pow(xsq, onep5);	/* zz = xsq ** onep5; */
		    z = x * xsq;
		}
		else
		{
		    y = dely * ran(i1) + c;
		    y2 = (y / TWO + y) - y;
		    y = y2 + y2;
		    z = pow(x, y);	/* z=x ** y; */
		    zz = pow(xsq, y2);	/* zz=xsq ** y2; */
		}
	    }
	    w = ONE;
	    if (z != ZERO)	/* */
		w = (z - zz) / z;
	    if (w > ZERO)
		k1 = k1 + 1;
	    if (w <= ZERO)
		k3 = k3 + 1;
	    w = ABS(w);
	    if (w > r6)
	    {
		r6 = w;
		x1 = x;
		if (j == 4 || j == 6)
		    y1 = y;
	    }
	    r7 = r7 + w * w;
	    xl = xl + del;
	}

	k2 = n - k3 - k1;
	r7 = sqrt(r7 / xn);
	if (j == 1)
	{
	    printf("\fTEST OF X**1.0 VS X\n\n\n");
	    printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n", n);
	    printf("      (" F15P4E "," F15P4E ")\n\n\n", a, b);
	    printf(" X**1.0 WAS LARGER%6d TIMES,\n", k1);
	    printf("            AGREED%6d TIMES, AND\n", k2);
	    printf("       WAS SMALLER%6d TIMES.\n\n\n", k3);
	}
	else if (j != 4 && j != 6)
	{
	    printf("\fTEST OF XSQ**1.5 VS XSQ*X\n\n\n");
	    printf("%6d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n", n);
	    printf("      (" F15P4E "," F15P4E ")\n\n\n", a, b);
	    printf(" X**1.5 WAS LARGER%6d TIMES,\n", k1);
	    printf("            AGREED%6d TIMES, AND\n", k2);
	    printf("       WAS SMALLER%6d TIMES.\n\n\n", k3);
	}
	else
	{
	    printf("\fTEST OF X**Y VS XSQ**(Y/2)\n\n\n");
	    w = c + dely;
	    printf(" %6d RANDOM ARGUMENTS WERE TESTED FROM THE REGION\n", n);
	    printf("      X IN (" F15P4E "," F15P4E "), Y IN (" F15P4E "," F15P4E ")\n\n\n",
		a, b, c, w);
	    printf(" X**Y  WAS LARGER%6d TIMES,\n", k1);
	    printf("           AGREED%6d TIMES, AND\n", k2);
	    printf("      WAS SMALLER%6d TIMES.\n\n\n", k3);
	}
	printf(
" THERE ARE %4d BASE %4d SIGNIFICANT DIGITS IN A FLOATING-POINT NUMBER\n\n\n",
	    it, ibeta);
	w = -999.0e0;
	if (r6 != ZERO)
	    w = ALOG(ABS(r6)) / albeta;
	if (j != 4 && j != 6)
	{
	    printf(" THE MAXIMUM RELATIVE ERROR OF " F15P4E " = %4d ** " F7P2F "\n",
		r6, ibeta, w);
	    printf("    OCCURRED FOR X =" F17P6E "\n", x1);
	}
	if (j == 4 || j == 6)
	{
	    printf(" THE MAXIMUM RELATIVE ERROR OF " F15P4E " = %4d ** " F7P2F "\n",
		r6, ibeta, w);
	    printf("    OCCURRED FOR X =" F17P6E " Y =" F17P6E "\n", x1, y1);
	}
	w = AMAX1(ait + w, ZERO);
	printf(
	    " THE ESTIMATED LOSS OF BASE%4d SIGNIFICANT DIGITS IS" F7P2F "\n\n\n",
	    ibeta, w);
	w = -999.0e0;
	if (r7 != ZERO)
	    w = ALOG(ABS(r7)) / albeta;
	printf(
	" THE ROOT MEAN SQUARE RELATIVE ERROR WAS " F15P4E " = %4d **" F7P2F "\n",
	    r7, ibeta, w);
	w = AMAX1(ait + w, ZERO);
	printf(
	    " THE ESTIMATED LOSS OF BASE%4d SIGNIFICANT DIGITS IS" F7P2F "\n\n\n",
	    ibeta, w);
	if (j != 1)
	{
	    b = 10.0e0L;
	    a = 0.01e0L;
	    if (j == 2)
	    {
		a = ONE;
		b = exp(alxmax / 3.0e0L);
#ifdef __BORLANDC__
#ifdef LDOUBLE
		/*
		 * Borland C powl does overflow without reason for
		 * to large arguments
		 */
		b *= 0.98L;
#endif
#endif
	    }
	    else if (j == 4)
	    {
		a = ONE;
		b = 1e100L;
	    }
	    else if (j == 5)
	    {
		a = 1e-2L;
		b = 10.0L;
		dely = 1e2L;
		c = -0.5*dely;
	    }
	}
    }

    /* special tests */

    printf("\fSPECIAL TESTS\n\n\n");
    printf(" THE IDENTITY  X ** Y = (1/X) ** (-Y)  WILL BE TESTED.\n\n");
    printf("        X              Y         (X**Y-(1/X)**(-Y)) / X**Y\n\n");
    b = 10.0e0L;

    for (i = 1; i <= 5; i++)
    {				/* DO I = 1, 5 	 */
	x = ran(i1) * b + ONE;
	y = ran(i1) * b + ONE;
	z = pow(x, y);		/* z=x ** y */
	zz = pow((ONE / x), -y);/* zz=(ONE/x) ** (-y) */
	w = (z - zz) / z;
	printf(F15P7E F15P7E "      " F15P7E "\n\n", x, y, w);
    }

    /* test of error returns */

    printf("\fTEST OF ERROR RETURNS\n\n\n");
    x = beta;
    y = (float) minexp;
    printf(" (" F14P7E ") ** (" F14P7E ") WILL BE COMPUTED.\n", x, y);
    printf(" THIS SHOULD NOT TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    z = pow(x, y);		/* Z = X ** Y */
    if (errno)
	perror("pow()");
    printf(" THE VALUE RETURNED IS " F15P4E "\n\n\n\n", z);
    y = (float) maxexp - 1;
    printf(" (" F14P7E ") ** (" F14P7E ") WILL BE COMPUTED.\n", x, y);
    printf(" THIS SHOULD NOT TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    z = pow(x, y);		/* z = X ** Y */
    if (errno)
	perror("pow()");
    printf(" THE VALUE RETURNED IS " F15P4E "\n\n\n\n", z);
    x = ZERO;
    y = TWO;
    printf(" (" F14P7E ") ** (" F14P7E ") WILL BE COMPUTED.\n", x, y);
    printf(" THIS SHOULD NOT TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    z = pow(x, y);		/* z = X ** Y */
    if (errno)
	perror("pow()");
    printf(" THE VALUE RETURNED IS " F15P4E "\n\n\n\n", z);
    x = -y;
    y = ZERO;
    printf(" (" F14P7E ") ** (" F14P7E ") WILL BE COMPUTED.\n", x, y);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    z = pow(x, y);		/* z = X ** Y */
    if (errno)
	perror("pow()");
    printf(" THE VALUE RETURNED IS " F15P4E "\n\n\n\n", z);
    y = TWO;
    printf(" (" F14P7E ") ** (" F14P7E ") WILL BE COMPUTED.\n", x, y);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    z = pow(x, y);		/* z = X ** Y */
    if (errno)
	perror("pow()");
    printf(" THE VALUE RETURNED IS " F15P4E "\n\n\n\n", z);
    x = ZERO;
    y = ZERO;
    printf(" (" F14P7E ") ** (" F14P7E ") WILL BE COMPUTED.\n", x, y);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    z = pow(x, y);		/* z = X ** Y */
    if (errno)
	perror("pow()");
    printf(" THE VALUE RETURNED IS " F15P4E "\n\n\n\n", z);
    printf(" THIS CONCLUDES THE TESTS\n");
    return;
}
