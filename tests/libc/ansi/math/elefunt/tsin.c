/* -*-C-*- tsin.c */

#include "elefunt.h"

/*#     PROGRAM to test sin/cos
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
#                          integer such that  float(ibeta)**minexp
#                          is a positive floating-point number
#                 eps    - the smallest positive floating-point
#                          number such that 1.0+eps != 1.0
#                 epsneg - the smallest positive floating-point
#                          number such that 1.0-epsneg != 1.0
#
#        ran(k) - a function subprogram returning random real
#                 numbers uniformly distributed over (0,1)
#
#
#     standard fortran subprograms required
#
#         abs, alog, amax1, cos, float, sin, sqrt
#
#
#     latest revision - december 6, 1979
#
#     author - w. j. cody
#              argonne national laboratory
#
#*/
#if 0
#ifdef LDOUBLE
static long double _sinl(long double x)
{
    static long double PIO4L   = 7.8539816339744830961566E-1L;
    static long double DP1 = 7.853981554508209228515625E-1L;
    static long double DP2 = 7.946627356147928367136046290398E-9L;
    static long double DP3 = 3.061616997868382943065164830688E-17L;
    static long double lossth = 5.49755813888e11L; /* 2^39 */
    long double y, z;
    int sign = 1;
    long j;

    if( x < 0 )
    {
	x = -x;
	sign = -1;
    }
    if( x > lossth )
    {
	return _MATH_NANL;
    }

    y = floorl( x/PIO4L ); /* integer part of x/PIO4 */
    z = y;
    j = z; /* convert to integer for tests on the phase angle */
    /* map zeros to origin */
    if( j & 1 )
    {
	j += 1;
	y += 1.0;
    }
    j = j & 07; /* octant modulo 360 degrees */
    /* reflect in x axis */
    if( j > 3)
    {
	sign = -sign;
	j -= 4;
    }
    /* Extended precision modular arithmetic */
    z = ((x - y * DP1) - y * DP2) - y * DP3;

    if( (j==1) || (j==2) )
    {
	y = cosl(z);
    }
    else
    {
	y = sinl(z);
    }

    if(sign < 0)
	y = -y;
    return(y);
}
#undef sin
#define sin _sinl
#endif
#endif

void
tsin()
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
        del,
        eps,
        epsneg,
        expon,
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
        y,
        z,
        zz;
    machar(&ibeta, &it, &irnd, &ngrd, &machep, &negep, &iexp, &minexp,
        &maxexp, &eps, &epsneg, &xmin, &xmax);
    beta = (float) ibeta;
    albeta = ALOG(beta);
    ait = (float) it;
    three = 3.0e0L;
    a = ZERO;
    b = 1.570796327e0L;
    c = b;
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
	    y = x / three;
	    y += x;
	    y -= x;
	    x = three * y;
	    if (j == 3)
	    {
		z = cos(x);
		zz = cos(y);
		w = ONE;
		if (z != ZERO)
		    w = (z + zz * (three - 4.0e0L * zz * zz)) / z;
	    }
	    else
	    {
		z = sin(x);
		zz = sin(y);
		w = ONE;
		if (z != ZERO)
		    w = (z - zz * (three - 4.0e0L * zz * zz)) / z;
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
	if (j == 3)
	{
	    printf("\fTEST OF COS(X) VS 4*COS(X/3)**3-3*COS(X/3)\n\n");
	    printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n",n);
	    printf("      (" F15P4E "," F15P4E ")\n\n\n", a, b);
	    printf(" COS(X) WAS LARGER%6d TIMES,\n", k1);
	    printf("            AGREED%6d TIMES, AND\n", k2);
	    printf("       WAS SMALLER%6d TIMES.\n\n", k3);
	}
	else
	{
	    printf("\fTEST OF SIN(X) VS 3*SIN(X/3)-4*SIN(X/3)**3\n\n");
	    printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n", n);
	    printf("      (" F15P4E "," F15P4E ")\n\n\n", a, b);
	    printf(" SIN(X) WAS LARGER%6d TIMES,\n", k1);
	    printf("            AGREED%6d TIMES, AND\n", k2);
	    printf("       WAS SMALLER%6d TIMES.\n\n", k3);
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
	a = 18.84955592e0L;
	if (j == 2)
	    a = b + c;
	b = a + c;
    }

    /* special tests */

    printf("\fSPECIAL TESTS\n\n");
    c = ONE / ipow(beta, (it / 2));
    z = (sin(a + c) - sin(a - c)) / (c + c);
    printf(" IF " F13P6E " IS NOT ALMOST 1.0E0,    SIN HAS THE WRONG PERIOD.\n\n",
	z);

    printf(" THE IDENTITY   SIN(-X) = -SIN(X)   WILL BE TESTED.\n");
    printf("     X        F(X) + F(-X)\n");

    for (i = 1; i <= 5; ++i)
    {
	x = ran(i1) * a;
	z = sin(x) + sin(-x);
	printf(F15P7E F15P7E "\n\n", x, z);
    }

    printf(" THE IDENTITY SIN(X) = X , X SMALL, WILL BE TESTED.\n");
    printf("    X         X - F(X)\n\n");
    betap = ipow(beta, it);
    x = ran(i1) / betap;

    for (i = 1; i <= 5; ++i)
    {
	z = x - sin(x);
	printf(F15P7E F15P7E "\n\n", x, z);
	x = x / beta;
    }

    printf(" THE IDENTITY   COS(-X) = COS(X)   WILL BE TESTED.\n");
    printf("        X         F(X) - F(-X)\n");

    for (i = 1; i <= 5; ++i)
    {
	x = ran(i1) * a;
	z = cos(x) - cos(-x);
	printf(F15P7E F15P7E "\n\n", x, z);
    }

    printf(" TEST OF UNDERFLOW FOR VERY SMALL ARGUMENT.\n\n");
    expon = (float) minexp *0.75e0L;
    x = pow(beta, expon);
    y = sin(x);
    printf("\n       SIN(" F13P6E ") = " F13P6E "\n", x, y);
    printf(" THE FOLLOWING THREE LINES ILLUSTRATE THE LOSS IN SIGNIFICANCE\n");
    printf(" FOR LARGE ARGUMENTS.  THE ARGUMENTS ARE CONSECUTIVE.\n");
    z = sqrt(betap);
    x = z * (ONE - epsneg);
    y = sin(x);
    printf("\n       SIN(" F13P6E ") =" F13P6E "\n", x, y);
    y = sin(z);
    printf("\n       SIN(" F13P6E ") =" F13P6E "\n", z, y);
    x = z * (ONE + eps);
    y = sin(x);
    printf("\n       SIN(" F13P6E ") =" F13P6E "\n", x, y);

    /* test of error returns */

    printf("\fTEST OF ERROR RETURNS\n\n\n");

    x = betap;
    printf(" SIN WILL BE CALLED WITH THE ARGUMENT" F15P4E "\n",x);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    y = sin(x);
    if (errno)
	perror("sin()");
    printf(" SIN RETURNED THE VALUE" F15P4E "\n\n\n\n", y);

    printf(" THIS CONCLUDES THE TESTS\n");
}
