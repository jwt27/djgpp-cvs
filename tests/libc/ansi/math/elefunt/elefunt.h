/* -*-C-*- elefunt.h */

/* Header file for Cody & Waite Elementary Function Test Package -- C Version */
/* Only this file needs to be #include'd by other files in the test package. */
/* See README file for details. */

#include <stdio.h>
#include <math.h>
#ifdef  LDOUBLE
#include <mathext.h>
#endif
#include <errno.h>
/* extern int errno; */			/* maybe "int errno" at your site */

#ifdef MSC
#include <float.h>			/* need for _control87() in machar.c */
#endif

#ifdef __TURBOC__
#include <float.h>			/* need for _control87() in machar.c */
#endif

#ifdef LDOUBLE
#undef float
#define float long double
#define fabs fabsl
#define sqrt sqrtl
#define cbrt cbrtl
#define exp expl
#ifdef __GO32__
#define exp2 pow2l
#define exp10 pow10l
#else
#define exp2(x) powl(2.0L, x)
#define exp10(x) powl(10.0L, x)
#endif
#define log logl
#define log10 log10l
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
#define atan2 atan2l
#define F15P4E "%16.4Le"
#define F15P7E "%16.7Le"
#define F14P7E "%15.7Le"
#define F15P5E "%16.5Le"
#define F17P6E "%18.6Le"
#define F13P6E "%15.6Le"
#define F7P2F "%7.2Lf"
#define F7P4F "%7.4Lf"
#else
#define float double
#ifdef __GO32__
#define exp2 pow2
#define exp10 pow10
#else
#define exp2(x) pow(2.0L, x)
#define exp10(x) pow(10.0L, x)
#endif
#define F15P4E "%15.4e"
#define F15P7E "%15.7e"
#define F14P7E "%14.7e"
#define F15P5E "%15.5e"
#define F17P6E "%17.6e"
#define F13P6E "%13.6e"
#define F7P2F "%7.2f"
#define F7P4F "%7.4f"
#endif



#define ONE 1.0L
#define ZERO 0.0L
#define TWO 2.0L
#define TEN 10.0L


#ifdef VMS

#ifndef EXIT_FAILURE
#define EXIT_FAILURE	((1 << 28) + 2) /* (suppresses %NONAME-E-NOMSG) */
#endif /* EXIT_FAILURE */

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS	(1)
#endif /* EXIT_SUCCESS */

#else /* NOT VMS */

#ifndef EXIT_FAILURE
#define EXIT_FAILURE	(1)
#endif /* EXIT_FAILURE */

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS	(0)
#endif /* EXIT_SUCCESS */

#endif /* VMS */


#ifdef PCC_20
typedef int void;
#endif

#if defined(__STDC__) || defined(__TURBOC__)
#define ARGS(parenthesized_list) parenthesized_list
#define VOIDP			void*

#else  /* NOT STDC */
/* Provide some synonyms for new ANSI C features */

#ifdef __GNUC__			/* GNU gcc supports ANSI prototypes */
#define ARGS(parenthesized_list) parenthesized_list
#else
#ifdef ardent			/* Ardent cc supports ANSI prototypes */
#define ARGS(parenthesized_list) parenthesized_list
#else
#define ARGS(parenthesized_list) ()
#endif /* ardent */
#endif /* __GNUC__ */
#endif /* __STDC__ */

extern void	machar ARGS((int* ibeta, int* it, int* irnd, int* ngrd,
			int* machep, int* negep, int* iexp, int* minexp,
			int* maxexp, float* eps, float* epsneg,
			float* xmin, float* xmax));
extern float	randl ARGS((float x));
extern float	ran ARGS((int k));

#if 0
/* This is lacking on most systems */
extern double	cotan ARGS((void));
#else
#define cotan(x) (ONE/tan(x))		/* not defined in most C libraries */
#endif

extern float	ipow ARGS((float x, int n));
extern void	talog ARGS((void));
extern void	tasin ARGS((void));
extern void	tatan ARGS((void));
extern void	texp ARGS((void));
extern void	texp2 ARGS((void));
extern void	texp10 ARGS((void));
extern void	tpower ARGS((void));
extern void	tsin ARGS((void));
extern void	tsinh ARGS((void));
extern void	tmacha ARGS((void));
extern void	tsqrt ARGS((void));
extern void	ttan ARGS((void));
extern void	ttanh ARGS((void));
extern void	store ARGS((float *));
extern void     init ARGS((void));

#ifdef abs
#undef abs
#endif
#define abs(x) (((x) < 0) ? -(x) : (x))

#ifdef ABS
#undef ABS
#endif
#define ABS(x) (((x) < 0) ? -(x) : (x))

#define AINT(x) ((float)((int)(x)))

#define alog(x) log(x)
#define ALOG(x) log(x)

#define alog10(x) log10(x)
#define ALOG10(x) log10(x)

#define amax1(x,y) (((x) >= (y)) ? (x) : (y))
#define AMAX1(x,y) (((x) >= (y)) ? (x) : (y))

#define amin1(x,y) (((x) <= (y)) ? (x) : (y))
#define AMIN1(x,y) (((x) <= (y)) ? (x) : (y))

#define DBLE(x) ((double)(x))

#define DOUBLE(x) ((double)(x))

#define FLOAT(x) ((float)(x))

#define iabs(x) (((x) < 0) ? -(x) : (x))
#define IABS(x) (((x) < 0) ? -(x) : (x))

#define INT(x) ((int)(x))

#define sign(x,y) (((y) >= 0) ? ABS(x) : -ABS(x))
#define SIGN(x,y) (((y) >= 0) ? ABS(x) : -ABS(x))
