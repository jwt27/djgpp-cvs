#include "elefunt.h"

#ifdef __STDC__
void store(float* p);
#else
void store();
#endif

void
machar(ibeta, it, irnd, ngrd, machep, negep, iexp, minexp, maxexp,
	eps, epsneg, xmin, xmax)
int *ibeta,
    *iexp,
    *irnd,
    *it,
    *machep,
    *maxexp,
    *minexp,
    *negep,
    *ngrd;
float *eps,
    *epsneg,
    *xmax,
    *xmin;
/***********************************************************************
#
#     THIS SUBROUTINE IS INTENDED TO DETERMINE THE CHARACTERISTICS
#     OF THE FLOATING-POINT ARITHMETIC SYSTEM THAT ARE SPECIFIED
#     BELOW.  THE FIRST THREE ARE DETERMINED ACCORDING TO AN
#     ALGORITHM DUE TO M. MALCOLM, CACM 15 (1972), PP. 949-951,
#     INCORPORATING SOME, BUT NOT ALL, OF THE IMPROVEMENTS
#     SUGGESTED BY M. GENTLEMAN AND S. MAROVICH, CACM 17 (1974),
#     PP. 276-277.  THE VERSION GIVEN HERE IS FOR SINGLE PRECISION.
#     CARDS CONTAINING  CD  IN COLUMNS 1 AND 2 CAN BE USED TO
#     CONVERT THE SUBROUTINE TO DOUBLE PRECISION BY REPLACING
#     EXISTING CARDS IN THE OBVIOUS MANNER.
#
#
#       IBETA   - THE RADIX OF THE FLOATING-POINT REPRESENTATION
#       IT      - THE NUMBER OF BASE IBETA DIGITS IN THE FLOATING-POINT
#                 SIGNIFICAND
#       IRND    - 0 IF FLOATING-POINT ADDITION CHOPS,
#                 1 IF FLOATING-POINT ADDITION ROUNDS
#       NGRD    - THE NUMBER OF GUARD DIGITS FOR MULTIPLICATION.  IT IS
#                 0 IF  IRND=1, OR IF  IRND=0  AND ONLY  IT  BASE  IBETA
#                   DIGITS PARTICIPATE IN THE POST NORMALIZATION SHIFT
#                   OF THE FLOATING-POINT SIGNIFICAND IN MULTIPLICATION
#                 1 IF  IRND=0  AND MORE THAN  IT  BASE  IBETA  DIGITS
#                   PARTICIPATE IN THE POST NORMALIZATION SHIFT OF THE
#                   FLOATING-POINT SIGNIFICAND IN MULTIPLICATION
#       MACHEP  - THE LARGEST NEGATIVE INTEGER SUCH THAT
#                 1.0+FLOAT(IBETA)**MACHEP .NE. 1.0, EXCEPT THAT
#                 MACHEP IS BOUNDED BELOW BY  -(IT+3)
#       NEGEPS  - THE LARGEST NEGATIVE INTEGER SUCH THAT
#                 1.0-FLOAT(IBETA)**NEGEPS .NE. 1.0, EXCEPT THAT
#                 NEGEPS IS BOUNDED BELOW BY  -(IT+3)
#       IEXP    - THE NUMBER OF BITS (DECIMAL PLACES IF IBETA = 10)
#                 RESERVED FOR THE REPRESENTATION OF THE EXPONENT
#                 (INCLUDING THE BIAS OR SIGN) OF A FLOATING-POINT
#                 NUMBER
#       MINEXP  - THE LARGEST IN MAGNITUDE NEGATIVE INTEGER SUCH THAT
#                 FLOAT(IBETA)**MINEXP IS A POSITIVE FLOATING-POINT
#                 NUMBER
#       MAXEXP  - THE LARGEST POSITIVE INTEGER EXPONENT FOR A FINITE
#                 FLOATING-POINT NUMBER
#       EPS     - THE SMALLEST POSITIVE FLOATING-POINT NUMBER SUCH
#                 THAT  1.0+EPS .NE. 1.0. IN PARTICULAR, IF EITHER
#                 IBETA = 2  OR  IRND = 0, EPS = FLOAT(IBETA)**MACHEP.
#                 OTHERWISE,  EPS = (FLOAT(IBETA)**MACHEP)/2
#       EPSNEG  - A SMALL POSITIVE FLOATING-POINT NUMBER SUCH THAT
#                 1.0-EPSNEG .NE. 1.0. IN PARTICULAR, IF IBETA = 2
#                 OR  IRND = 0, EPSNEG = FLOAT(IBETA)**NEGEPS.
#                 OTHERWISE,  EPSNEG = (IBETA**NEGEPS)/2.  BECAUSE
#                 NEGEPS IS BOUNDED BELOW BY -(IT+3), EPSNEG MAY NOT
#                 BE THE SMALLEST NUMBER WHICH CAN ALTER 1.0 BY
#                 SUBTRACTION.
#       XMIN    - THE SMALLEST NON-VANISHING FLOATING-POINT POWER OF THE
#                 RADIX.  IN PARTICULAR,  XMIN = FLOAT(IBETA)**MINEXP
#       XMAX    - THE LARGEST FINITE FLOATING-POINT NUMBER.  IN
#                 PARTICULAR   XMAX = (1.0-EPSNEG)*FLOAT(IBETA)**MAXEXP
#                 NOTE - ON SOME MACHINES  XMAX  WILL BE ONLY THE
#                 SECOND, OR PERHAPS THIRD, LARGEST NUMBER, BEING
#                 TOO SMALL BY 1 OR 2 UNITS IN THE LAST DIGIT OF
#                 THE SIGNIFICAND.
#
#     LATEST REVISION - OCTOBER 22, 1979
#
#     AUTHOR - W. J. CODY
#              ARGONNE NATIONAL LABORATORY
#
***********************************************************************/
{
    int i,
        iz,
        j,
        k,
        mx;
    float a,
        b,
        beta,
        betain,
        betam1,
        y,
        z;
    float t1,
        t2,
        t3;			/* temporaries for subexpressions */

#ifdef MSC
    /* Need to allow for overflow here--MSC otherwise aborts job */
    _control87(CW_DEFAULT | EM_OVERFLOW, 0xffff);
#endif

#ifdef __TURBOC__
   /* Get around DOS 3.2 bug that crashes PC after 8 floating-point
   traps, as described in README file for Turbo C 1.5 distribution.
   This call disables all floating-point traps. */
   _control87(MCW_EM, MCW_EM);
#if 0
   _control87(IC_AFFINE,IC_AFFINE);	/* Turbo C 1.5 uses IC_PROJECTIVE */
					/* Microsoft C 5.0 uses IC_AFFINE */
#endif
#endif


    /* determine *ibeta,beta ala Malcolm */

    a = ONE;
    do
    {
	a = a + a;
	t1 = a + ONE;
	t2 = t1 - a;
	t3 = t2 - ONE;
#ifdef DEBUG_MACHAR
	printf("L1: %15.7e %15.7e %15.7e %15.7e\n",a,t1,t2,t3);
#endif
    }
    while (t3 == ZERO);

    b = ONE;
    do
    {
	b = b + b;
    	t1 = a + b;
	t2 = t1 - a;
#ifdef DEBUG_MACHAR
	printf("L2: %15.7e %15.7e %15.7e\n",b,t1,t2);
#endif
    }
    while (t2 == ZERO);

    t1 = a + b;
    *ibeta = INT(t1 - a);
    beta = FLOAT(*ibeta);

    /* determine *it, *irnd */
    *it = 0;
    b = ONE;
    do
    {
	(*it)++;
	b = b * beta;
	t1 = b + ONE;
	t2 = t1 - b;
	t3 = t2 - ONE;
#ifdef DEBUG_MACHAR
	printf("L3: %15.7e %15.7e %15.7e %15.7e\n",b,t1,t2,t3);
#endif
    }
    while (t3 == ZERO);

    *irnd = 0;
    betam1 = beta - ONE;
    t1 = a + betam1;
    t2 = t1 - a;
    if (t2 != ZERO)
	*irnd = 1;

    /* determine *negep, *epsneg */

    *negep = *it + 3;
    betain = ONE / beta;
    a = ONE;

    for (i = 1; i <= *negep; ++i)
	a = a * betain;

    b = a;
    for (;;)
    {
	t1 = ONE - a;
    	store(&t1);
	t2 = t1 - ONE;
    	store(&t2);
	if (t2 != ZERO)
	    break;
	a = a * beta;
    	store(&a);
	(*negep)--;
#ifdef DEBUG_MACHAR
	printf("L4: %15.7e %15.7e %15.7e %15.7e\n",a,t1,t2,t3);
#endif
    }

    *negep = -*negep;
    *epsneg = a;
    if (!((*ibeta == 2) || (*irnd == 0)))
    {
	a = (a * (ONE + a)) / (ONE + ONE);
    	store(&a);
	t1 = ONE - a;
    	store(&t1);
	t2 = t1 - ONE;
    	store(&t2);
	if (t2 != ZERO)
	    *epsneg = a;
    }

    /* determine *machep, *eps */

    *machep = -*it - 3;
    a = b;
    for (;;)
    {
	t1 = ONE + a;
    	store(&t1);
	t2 = t1 - ONE;
    	store(&t2);
	if (t2 != ZERO)
	    break;
	a = a * beta;
    	store(&a);
	(*machep)++;
#ifdef DEBUG_MACHAR
	printf("L5: %15.7e %15.7e %15.7e %15.7e\n",a,t1,t2);
#endif
    }
    *eps = a;
    if (!((*ibeta == 2) || (*irnd == 0)))
    {
	a = (a * (ONE + a)) / (ONE + ONE);
    	store(&a);
	t1 = ONE + a;
    	store(&t1);
	t2 = t1 - ONE;
    	store(&t2);
	if (t2 != ZERO)
	    *eps = a;
    }

    /* determine *ngrd */
    *ngrd = 0;
    t1 = ONE + *eps;
    store(&t1);
    t2 = t1 * ONE;
    store(&t2);
    t3 = t2 - ONE;
    store(&t3);
    if ((*irnd == 0) && (t3 != ZERO))
	*ngrd = 1;

    /* Determine *iexp, *minexp, *xmin.
       Loop to determine largest i and k = 2**i such that (1/beta) ** (2**(i))
       does not underflow. Exit from loop is signaled by an underflow. */

    i = 0;
    k = 1;
    z = betain;
    for (;;)
    {
	y = z;
	store(&y);
	z = y * y;
    	store(&z);

	/* check for underflow here */
	a = z * ONE;
    	store(&a);
	t1 = a + a;
	store(&t1);
	if ((t1 == ZERO) || (ABS(z) >= y))
	    break;
	i++;
	k = k + k;
#ifdef DEBUG_MACHAR
	printf("L6: %15.7e %15.7e %15.7e %15.7e %d\n",a,t1,y,z,k);
#endif
    }
    if (*ibeta != 10)
    {
	*iexp = i + 1;
	mx = k + k;
    }
    else
    {				/* for decimal machines only */
	*iexp = 2;
	iz = *ibeta;
	while (k >= iz)
	{
	    iz = iz * *ibeta;
	    (*iexp)++;
	}
	mx = iz + iz - 1;
    }
    for (;;)
    {	/* loop to determine *minexp, *xmin
           Exit from loop is signaled by an underflow. */
	*xmin = y;
	y = y * betain;
    	store(&y);

	/* check for underflow here */
	a = y * ONE;
    	store(&a);
	if (((a + a) == ZERO) || (ABS(y) >= *xmin))
	    break;
	k++;

#ifdef DEBUG_MACHAR
	printf("L7: %15.7e %15.7e %d\n",a,y,k);
#endif

#ifdef __TURBOC__
	/* Turbo C 1.5 computes minexp = -1074, but the run-time
	library turns 2**p (p < -1023) into 0.0, so we reset the
	limit so as to prevent the use of denormalized numbers.
	Microsoft C 5.0 correctly generates denormalized numbers down to
	2**(-1074). */
	if (k >= 1023)
	    break;
#endif
    }
    *minexp = -k;

    /* determine *maxexp, *xmax */
    if (!((mx > (k + k - 3)) || (*ibeta == 10)))
    {
	mx = mx + mx;
	(*iexp)++;
    }
    *maxexp = mx + *minexp;
    /* adjust for machines with implicit leading bit in binary significand */
    /* and machines with radix point at extreme right of significand */

    i = *maxexp + *minexp;
    if ((*ibeta == 2) && (i == 0))
	*maxexp = *maxexp - 1;
    if (i > 20)
	*maxexp = *maxexp - 1;
    if (a != y)
	*maxexp = *maxexp - 2;

#ifdef DEBUG_MACHAR
	printf("L7a: minexp = %d\tmaxexp = %d\n",*minexp,*maxexp);
#endif

    *xmax = ONE - *epsneg;
    store(xmax);
    if (*xmax * ONE != *xmax)
	*xmax = ONE - beta * *epsneg;
    store(xmax);
    *xmax = *xmax / (beta * beta * beta * *xmin);
    store(xmax);
    i = *maxexp + *minexp + 3;
    for (j = 1; j <= i; ++j)
    {
        if (*ibeta == 2)
	    *xmax = *xmax + *xmax;
        else
	    *xmax = *xmax * beta;
	store(xmax);
#ifdef DEBUG_MACHAR
#ifdef IEEE
        /* See remark below */
#else
	printf("L8: %15.7e %d\n",*xmax,j);
#endif
#endif
    }

#ifdef IEEE
    /* On a host with IEEE arithmetic, the above code computes 
       *minexp = -1074 (sp: -149) (with gradual underflow; else
       -1023 (sp: -126)), which is acceptable, but it finds
       *maxexp = 3021 (sp: 362?) (with gradual underflow),
       instead of 1024 (sp: 128?)  because the code fragment

       if (!((mx > (k + k - 3)) || (*ibeta == 10)))
       {
	  mx = mx + mx;
	  (*iexp)++;
       }

       doubles mx from 2048 to 4096 (sp: 256 to 512).  The
       following code simply compares z with (z*beta)/beta, with
       z = beta**k, until it finds a mismatch, at which point we
       have found the real *maxexp. */
       
    z = beta;
    *maxexp = 1;
    for (;;)
    {
	if (z != z)
	{
	  printf("?ERROR: z*beta has generated a NaN instead of infinity!\n");
	  break;
	}
	y = z * beta;
	if (y == z)
	    break;
	if ((y / beta) != z)
	    break;
	(*maxexp)++;
	z = y;
#ifdef DEBUG_MACHAR
	printf("L9: %15.7e %d\n",z,*maxexp);
#endif
    }
    z *= (ONE - *epsneg);
    store(&z);
    z *= beta;
    store(&z);
    *xmax = z;
#endif
    return;
}
