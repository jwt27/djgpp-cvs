/* -*-C-*- tsqrt.c */

#include "elefunt.h"

/*    PROGRAM TO TEST SQRT
#
#     DATA REQUIRED
#
#        NONE
#
#     SUBPROGRAMS REQUIRED FROM THIS PACKAGE
#
#        MACHAR - AN ENVIRONMENTAL INQUIRY PROGRAM PROVIDING
#                 INFORMATION ON THE FLOATING-POINT ARITHMETIC
#                 SYSTEM.  NOTE THAT THE CALL TO MACHAR CAN
#                 BE DELETED PROVIDED THE FOLLOWING SIX
#                 PARAMETERS ARE ASSIGNED THE VALUES INDICATED
#
#                 IBETA  - THE RADIX OF THE FLOATING-POINT SYSTEM
#                 IT     - THE NUMBER OF BASE-IBETA DIGITS IN THE
#                          SIGNIFICAND OF A FLOATING-POINT NUMBER
#                 EPS    - THE SMALLEST POSITIVE FLOATING-POINT
#                          NUMBER SUCH THAT 1.0+EPS .NE. 1.0
#                 EPSNEG - THE SMALLEST POSITIVE FLOATING-POINT
#                          NUMBER SUCH THAT 1.0-EPSNEG .NE. 1.0
#                 XMIN   - THE SMALLEST NON-VANISHING FLOATING-POINT
#                          POWER OF THE RADIX
#                 XMAX   - THE LARGEST FINITE FLOATING-POINT NO.
#
#      RANDL(X) - A FUNCTION SUBPROGRAM RETURNING LOGARITHMICALLY
#                 DISTRIBUTED RANDOM REAL NUMBERS.  IN PARTICULAR,
#                        A * RANDL(ALOG(B/A))
#                 IS LOGARITHMICALLY DISTRIBUTED OVER (A,B)
#
#        RAN(K) - A FUNCTION SUBPROGRAM RETURNING RANDOM REAL
#                 NUMBERS UNIFORMLY DISTRIBUTED OVER (0,1)
#
#
#     STANDARD FORTRAN SUBPROGRAMS REQUIRED
#
#         ABS, ALOG, AMAX1, FLOAT, SQRT
#
#
#     LATEST REVISION - AUGUST 2, 1979
#
#     AUTHOR - W. J. CODY
#              ARGONNE NATIONAL LABORATORY
#
*****************************************************************************/

void
tsqrt()
{
    int i,
        ibeta,
        iexp,
        irnd,
        it,
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
        c,
        eps,
        epsneg,
        r6,
        r7,
        sqbeta,
        w,
	x,
        xmax,
        xmin,
        xn,
        x1,
        y,
        z;

    /*******************************************************************/

    machar(&ibeta, &it, &irnd, &ngrd, &machep, &negep, &iexp, &minexp,
	   &maxexp, &eps, &epsneg, &xmin, &xmax);
    beta = (float) ibeta;
    sqbeta = sqrt(beta);
    albeta = ALOG(beta);
    ait = (float) it;
    a = ONE / sqbeta;
    b = ONE;
    n = 2000;
    xn = (float) n;

    /* random argument accuracy tests */

    for (j = 1; j <= 2; ++j)
    {
	c = ALOG(b / a);
	k1 = 0;
	k3 = 0;
	x1 = ZERO;
	r6 = ZERO;
	r7 = ZERO;

	for (i = 1; i <= n; ++i)
	{
	    x = a * randl(c);
	    y = x * x;
	    z = sqrt(y);
	    w = (z - x) / x;
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
	}

	k2 = n - k1 - k3;
	r7 = sqrt(r7 / xn);
	printf("\fTEST OF SQRT(X*X) - X\n\n\n");
	printf("%7d RANDOM ARGUMENTS WERE TESTED FROM THE INTERVAL\n", n);
	printf("       (" F15P4E "," F15P4E ")\n\n\n", a, b);
	printf(" SQRT(X) WAS LARGER%6d TIMES,\n", k1);
	printf("             AGREED%6d TIMES, AND\n", k2);
	printf("        WAS SMALLER%6d TIMES.\n\n\n", k3);
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
	a = ONE;
	b = sqbeta;
    }

    /* special tests */

    printf("\fTEST OF SPECIAL ARGUMENTS\n\n\n");
    x = xmin;
    y = sqrt(x);
    printf(" SQRT(XMIN) = SQRT(" F15P7E ") = " F15P7E "\n\n\n", x, y);
    x = ONE - epsneg;
    y = sqrt(x);
    printf(" SQRT(1-EPSNEG) = SQRT(1-" F15P7E ") = " F15P7E "\n\n\n", epsneg, y);
    x = ONE;
    y = sqrt(x);
    printf(" SQRT(1.0) = SQRT(" F15P7E ") = " F15P7E "\n\n\n", x, y);
    x = ONE + eps;
    y = sqrt(x);
    printf(" SQRT(1+EPS) = SQRT(1+" F15P7E ") = " F15P7E "\n\n\n", eps, y);
    x = xmax;
    y = sqrt(x);
    printf(" SQRT(XMAX) = SQRT(" F15P7E ") = " F15P7E "\n\n\n", x, y);

    /* test of error returns */

    printf("\fTEST OF ERROR RETURNS\n\n\n");

    x = ZERO;
    printf(" SQRT WILL BE CALLED WITH THE ARGUMENT " F15P4E "\n", x);
    printf(" THIS SHOULD NOT TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    y = sqrt(x);
    if (errno)
	perror("sqrt()");
    printf(" SQRT RETURNED THE VALUE " F15P4E "\n\n\n", y);

    x = -ONE;
    printf("\nSQRT WILL BE CALLED WITH THE ARGUMENT " F15P4E "\n", x);
    printf(" THIS SHOULD TRIGGER AN ERROR MESSAGE\n\n\n");
    fflush(stdout);
    errno = 0;
    y = sqrt(x);
    if (errno)
	perror("sqrt()");
    printf(" SQRT RETURNED THE VALUE " F15P4E "\n\n\n\n", y);

    printf(" THIS CONCLUDES THE TESTS\n");
}
