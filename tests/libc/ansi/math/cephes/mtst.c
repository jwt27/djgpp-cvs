/*   mtst.c
 Consistency tests for math functions.
 Inverse function tests and Wronksians for random arguments.
 With NTRIALS=10000, the following are typical results for
 IEEE double precision arithmetic:

Consistency test of math functions.
Max and rms relative errors for 10000 random arguments.
x =   cbrt(   cube(x) ):  max = 1.54E-016   rms = 6.50E-018
x =   atan(    tan(x) ):  max = 2.83E-016   rms = 7.68E-017
x =    sin(   asin(x) ):  max = 4.44E-016   rms = 7.01E-017
x =   sqrt( square(x) ):  max = 1.57E-016   rms = 2.70E-017
x =    log(    exp(x) ):  max = 1.88E-014   rms = 2.02E-016
x =   tanh(  atanh(x) ):  max = 4.00E-016   rms = 8.29E-017
x =  asinh(   sinh(x) ):  max = 2.04E-016   rms = 1.56E-017
x =  acosh(   cosh(x) ):  max = 2.67E-014   rms = 2.95E-016
x = pow( pow(x,a),1/a ):  max = 3.38E-012   rms = 3.39E-014
Legendre  ellpk,  ellpe:  max = 1.55E-011   rms = 1.74E-013
x =  ndtri(   ndtr(x) ):  max = 4.09E-014   rms = 6.40E-016
Absolute error criterion (but relative if >1):
lgam(x) = log(gamma(x)):  max = 4.49E-016   rms = 8.80E-017
x =    cos(   acos(x) ):  max = 4.44E-016   rms = 1.00E-016
Absolute error and only 2000 trials:
Wronksian of   Yn,   Jn:  max = 7.12E-011   rms = 1.59E-012
Wronksian of   Kn,   Iv:  max = 7.07E-012   rms = 5.25E-013

*/

/*
Cephes Math Library Release 2.1:  December, 1988
Copyright 1984, 1987, 1988 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
f*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#ifdef  LDOUBLE
#include <mathext.h>
#endif
#include "mconf.h"

#define NTRIALS 10000
#define WTRIALS (NTRIALS/5)
#define STRTST 0

#ifdef LDOUBLE
#define double long double
#undef fabs
#define fabs fabsl
#define sqrt sqrtl
#define cbrt cbrtl
#define exp expl
#define log logl
#define tan tanl
#define atan atanl
#define sin sinl
#define asin asinl
#define cos cosl
#define acos acosl
#define pow powl
#define tanh tanhl
#define atanh atanhl
#define sinh sinhl
#define asinh asinhl
#define cosh coshl
#define acosh acoshl
#define drand drandl
#define pow2 pow2l
#define log2 log2l
#define pow10 pow10l
#define log10 log10l
#define MINLOG MINLOGL
#define MAXLOG MAXLOGL
#define LOG2E  LOG2EL
#define CPI    PIL
#define PIO2   PIO2L
#define GFORMAT "%Lg"
#define P18EFORMAT "%.20LE"
#define P4EFORMAT "%.4LE"
#define P2EFORMAT "%.2LE"
#else
#define GFORMAT "%g"
#define P18EFORMAT "%.18E"
#define P4EFORMAT "%.4E"
#define P2EFORMAT "%.2E"
#ifdef  __DJGPP__
#include <float.h>
#define _set_cw87(x)    _control87(x, 0xffffffffU)
#endif
#endif

/*
double          ndtr(), ndtri(), ellpe(), ellpk(), gamma();
double          fabs(), sqrt();
double          cbrt(), exp(), log(), tan(), atan();
double          sin(), asin(), cos(), acos(), pow();
double          tanh(), atanh(), sinh(), asinh(), cosh(), acosh();
double          jn(), yn(), iv(), kn();
*/

/* Provide inverses for square root and cube root: */
double
square(x)
    double          x;
{
    return (x * x);
}

double
cube(x)
    double          x;
{
    return (x * x * x);
}

/* lookup table for each function */
struct fundef
{
    char           *nam1;	/* the function */
    double          (*name) ();
    char           *nam2;	/* its inverse  */
    double          (*inv) ();
    int             nargs;	/* number of function arguments */
    int             tstyp;	/* type code of the function */
    long            ctrl;	/* relative error flag */
    double          arg1w;	/* width of domain for 1st arg */
    double          arg1l;	/* lower bound domain 1st arg */
    long            arg1f;	/* flags, e.g. integer arg */
    double          arg2w;	/* same info for args 2, 3, 4 */
    double          arg2l;
    long            arg2f;
/*
	double arg3w;
	double arg3l;
	long arg3f;
	double arg4w;
	double arg4l;
	long arg4f;
*/
};


/* fundef.ctrl bits: */
#define RELERR 1
#define EXPSCAL 4

/* fundef.tstyp  test types: */
#define POWER 1
#define ELLIP 2
#define GAMMA 3
#define WRONK1 4
#define WRONK2 5
#define WRONK3 6

/* fundef.argNf  argument flag bits: */
#define INT 2

extern double   MINLOG;
extern double   MAXLOG;
extern double   LOG2E;
extern double   CPI;
extern double   PIO2;

/*
define MINLOG -170.0
define MAXLOG +170.0
define PI 3.14159265358979323846
define PIO2 1.570796326794896619
*/

/*
#ifndef LDOUBLE
#define NTESTS 13
#else
#define NTESTS 11
#endif
*/

struct fundef   defs[] =
{
    {"   tan", tan, "  atan", atan, 1, 0, 1, 0.0, 0.0, 0,
     0.0, 0.0, 0},
    {"  asin", asin, "   sin", sin, 1, 0, 1, 2.0, -1.0, 0,
     0.0, 0.0, 0},
    {"square", square, "  sqrt", sqrt, 1, 0, 1, 170.0, -85.0, EXPSCAL,
     0.0, 0.0, 0},
    {"   exp", exp, "   log", log, 1, 0, 1, 340.0, -170.0, 0,
     0.0, 0.0, 0},
    {" atanh", atanh, "  tanh", tanh, 1, 0, 1, 2.0, -1.0, 0,
     0.0, 0.0, 0},
    {"  sinh", sinh, " asinh", asinh, 1, 0, 1, 340.0, 0.0, 0,
     0.0, 0.0, 0},
    {"  cosh", cosh, " acosh", acosh, 1, 0, 1, 340.0, 0.0, 0,
     0.0, 0.0, 0},
    {"pow", pow, "pow", pow, 2, POWER, 1, 25.0, 0.0, 0,
     50.0, -25.0, 0},
    {"  exp2", pow2, "  log2", log2, 1, 0, 1, 340.0, -170.0, 0,
     0.0, 0.0, 0},
    {" exp10", pow10, " log10", log10, 1, 0, 1, 340.0, -170.0, 0,
     0.0, 0.0, 0},
#if 0
    {" ellpe",  ellpe,   " ellpk",  ellpk, 1, ELLIP, 1, 1.0,    0.0,  0,
     0.0, 0.0, 0},
    {"  ndtr",   ndtr,   " ndtri",  ndtri, 1, 0, 1,   10.0,   -10.0,   0,
     0.0, 0.0, 0},
#endif
    {"  acos", acos, "   cos", cos, 1, 0, 0, 2.0, -1.0, 0,
     0.0, 0.0, 0},
#if 0
    {"  Iv",     iv,   "  Kn",     kn, 2, WRONK2, 0,  30.0, 0.1,  0,
     20.0, 0.0, INT},
#endif
    {NULL, NULL, NULL, NULL, 0, 0, 0,0.0, 0.0, 0, 0.0, 0.0, 0}
};

static char    *headrs[] =
{
    "x = %s( %s(x) ): ",
    "x = %s( %s(x,a),1/a ): ",	/* power */
    "Legendre %s, %s: ",	/* ellip */
    "%s(x) = log(%s(x)): ",	/* gamma */
    "Wronksian of %s, %s: ",
    "Wronksian of %s, %s: ",
    "Wronksian of %s, %s: "
};

static double   cy1 = 0.0;
static double   y2 = 0.0;
static double   y3 = 0.0;
static double   y4 = 0.0;
static double   a = 0.0;
static double   x = 0.0;
static double   y = 0.0;
static double   z = 0.0;
static double   e = 0.0;
static double   max = 0.0;
static double   rmsa = 0.0;
static double   rms = 0.0;
static double   ave = 0.0;

void mysig(int sig);
int
main()
{
    double          (*fun) ();
    double          (*ifun) ();
    struct fundef  *d;
    int             i, k=0, itst;
    int             m, ntr;
    _set_cw87(0x137f);
    ntr = NTRIALS;
    printf("Consistency test of math functions.\n");
    printf("Max and rms relative errors for %d random arguments.\n",
	   ntr);

/* Initialize machine dependent parameters: */
#ifdef __TURBOC__
#define FACT 0.99
#else
#define FACT 1.0000
#endif
    defs[0].arg1w = CPI;                /* tan */
    defs[0].arg1l = -CPI/2.0;
    defs[2].arg1w = MAXLOG*FACT;        /* sqrt */
    defs[2].arg1l = -MAXLOG/2.0*FACT;
    defs[3].arg1w = 2.0*MAXLOG*FACT;    /* exp */
    defs[3].arg1l = -MAXLOG*FACT;
    defs[5].arg1w = 2.0*MAXLOG*FACT;    /* sinh */
    defs[5].arg1l = -MAXLOG*FACT;
    defs[6].arg1w = MAXLOG*FACT;        /* cosh */
    defs[6].arg1l = 0.0;
    defs[8].arg1w = 2.0*MAXLOG*FACT*LOG2E;    /* exp2 */
    defs[8].arg1l = -MAXLOG*FACT*LOG2E;
    defs[9].arg1w = 2.0*MAXLOG*FACT*0.43429448L;    /* exp10 */
    defs[9].arg1l = -MAXLOG*FACT*0.43429448L;


/* Outer loop, on the test number: */

    for (itst = STRTST; defs[itst].name; itst++)
    {
	d = &defs[itst];
	m = 0;
	max = 0.0;
	rmsa = 0.0;
	ave = 0.0;
	fun = d->name;
	ifun = d->inv;

/* Absolute error criterion starts with gamma function
 * (put all such at end of table)
 */
	if (d->tstyp == GAMMA)
	    printf("Absolute error criterion (but relative if >1):\n");

/* Smaller number of trials for Wronksians
 * (put them at end of list)
 */
	if (d->tstyp == WRONK1)
	{
	    ntr = WTRIALS;
	    printf("Absolute error and only %d trials:\n", ntr);
	}

	printf(headrs[d->tstyp], d->nam2, d->nam1);
	printf("random arguments x in " GFORMAT " " GFORMAT " \n",
		d->arg1l, d->arg1l + d->arg1w);
	if (d->nargs == 2)
	    printf("random arguments a in " GFORMAT " " GFORMAT " \n",
		d->arg2l, d->arg2l + d->arg2w);

	for (i = 0; i < ntr; i++)
	{
	    m++;

/* make random number(s) in desired range(s) */
	    switch (d->nargs)
	    {

		default:
		    goto illegn;

		case 2:
		    drand(&a);
		    a = d->arg2w * (a - 1.0) + d->arg2l;
		    if (d->arg2f & EXPSCAL)
		    {
			a = exp(a);
			drand(&y2);
			a -= 1.0e-13 * a * y2;
		    }
		    if (d->arg2f & INT)
		    {
			k = a + 0.25;
			a = k;
		    }

		case 1:
		    drand(&x);
		    x = d->arg1w * (x - 1.0) + d->arg1l;
		    if (d->arg1f & EXPSCAL)
		    {
			x = exp(x);
			drand(&a);
			x += 1.0e-13 * x * a;
		    }
	    }


/* compute function under test */
	    switch (d->nargs)
	    {
		case 1:
		    switch (d->tstyp)
		    {
			case ELLIP:
			    cy1 = (*(fun)) (x);
			    y2 = (*(fun)) (1.0 - x);
			    y3 = (*(ifun)) (x);
			    y4 = (*(ifun)) (1.0 - x);
			    break;

			case GAMMA:
			    break;

			default:
			    z = (*(fun)) (x);
			    y = (*(ifun)) (z);
		    }
		    break;

		case 2:
		    if (d->arg2f & INT)
		    {
			switch (d->tstyp)
			{
			    case WRONK1:
				cy1 = (*fun) (k, x);	/* jn */
				y2 = (*fun) (k + 1, x);
				y3 = (*ifun) (k, x);	/* yn */
				y4 = (*ifun) (k + 1, x);
				break;

			    case WRONK2:
				cy1 = (*fun) (a, x);	/* iv */
				y2 = (*fun) (a + 1.0, x);
				y3 = (*ifun) (k, x);	/* kn */
				y4 = (*ifun) (k + 1, x);
				break;

			    default:
				z = (*fun) (k, x);
				y = (*ifun) (k, z);
			}
		    }
		    else
		    {
			if (d->tstyp == POWER)
			{
			    z = (*fun) (x, a);
			    y = (*ifun) (z, 1.0 / a);
			}
			else
			{
			    z = (*fun) (a, x);
			    y = (*ifun) (a, z);
			}
		    }
		    break;


		default:
		  illegn:
		    printf("Illegal nargs= %d", d->nargs);
		    exit(1);
	    }

	    switch (d->tstyp)
	    {
		case WRONK1:
		    e = (y2 * y3 - cy1 * y4) - 2.0 / (CPI * x);	/* Jn, Yn */
		    break;

		case WRONK2:
		    e = (y2 * y3 + cy1 * y4) - 1.0 / x;	/* In, Kn */
		    break;

		case ELLIP:
		    e = (cy1 - y3) * y4 + y3 * y2 - PIO2;
		    break;

		default:
		    e = y - x;
		    break;
	    }

	    if (d->ctrl & RELERR)
		e /= x;
	    else
	    {
		if (fabs(x) > 1.0)
		    e /= x;
	    }

	    ave += e;
/* absolute value of error */
	    if (e < 0)
		e = -e;

/* peak detect the error */
	    if (e > max)
	    {
		max = e;

		if (e > 1.0e-10)
		{
		    /* printf("x %.6E z %.6E y %.6E max %.4E\n",
			   x, z, y, max); */
		    printf("x " P18EFORMAT " z "  P18EFORMAT
			   " y " P18EFORMAT " max " P4EFORMAT " \n",
			   x, z, y, max);
		    if (d->tstyp >= WRONK1)
		    {
			printf("cy1 " P4EFORMAT
				" y2 " P4EFORMAT
				" y3 " P4EFORMAT
				" y4 " P4EFORMAT
				" k %d x " P4EFORMAT "\n",
			       cy1, y2, y3, y4, k, x);
		    }
		}

/*
	printf("%.8E %.8E %.4E %6ld \n", x, y, max, n);
	printf("%d %.8E %.8E %.4E %6ld \n", k, x, y, max, n);
	printf("%.6E %.6E %.6E %.4E %6ld \n", a, x, y, max, n);
	printf("%.6E %.6E %.6E %.6E %.4E %6ld \n", a, b, x, y, max, n);
	printf("%.4E %.4E %.4E %.4E %.4E %.4E %6ld \n",
		a, b, c, x, y, max, n);
*/
	    }

/* accumulate rms error	*/
	    e *= 1.0e16;	/* adjust range */
	    rmsa += e * e;	/* accumulate the square of the error */
	}

/* report after NTRIALS trials */
	rms = 1.0e-16 * sqrt(rmsa / m);
	printf(" max = " P2EFORMAT " rms = " P2EFORMAT "\n", max, rms);
    }				/* loop on itst */
    return(0);
}
