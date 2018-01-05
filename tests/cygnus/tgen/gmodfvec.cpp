// --------------------------------------------------------
// genmodfv - generates test vectors for modf() & modff()
// --------------------------------------------------------
#include <errno.h>
#include <float.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <qfloat.h>
#include "test.h"
#define USE_WRITE_2_VECTOR
#include "genmathv.h"

/* INDENT OFF */

LOOP_LIMITS  CtlsX[] =
    {	/* Start   Step     Stop  */
	{  -81,    .81,	   +81},
    };

/* INDENT ON */

/* ----------------------------------------------------------------------- */
/* MyModf - Returns Fraction part of double Mixed No., stores Integer Part */
/* ----------------------------------------------------------------------- */
double
MyModf(double Mixed, double *Int)
{
    if (Mixed >= 0)
    {
	*Int = floor(Mixed);
    }
    else
    {
	*Int = ceil(Mixed);
    }
    return copysign(Mixed - *Int, Mixed);
}

#define	DBL_MEAS_BIT	62
#define	FLT_MEAS_BIT	34

// -------------------------------------------------------------------
// GenDblVector - Generates Test Vectors for Double Precision Function
// -------------------------------------------------------------------
void
GenDblVector(char *Name)
{
    unsigned J;

    volatile  double  Mixed, Expected;

    // Redirect stdout to disk file

    AssignOutputFile(Name);

    // Generate first two lines of file

    GEN_FIRST_TWO_LINES

    // Generate vectors

    for (J = 0; J < (sizeof(CtlsX) / sizeof(CtlsX[0])); ++J)
    {
	for (Mixed = CtlsX[J].Start; Mixed <= CtlsX[J].Stop;
		Mixed += CtlsX[J].Step)
	{
	    double  Frac, Int;

	    Frac = MyModf(Mixed, &Int);

	    if (Frac <= DBL_MAX)
	    {
		errno = 0;
		Expected = Frac;
	    }
	    else
	    {
		errno = ERANGE;
		Expected = infinity();
		_fpreset();
	    }
	    Write2Vector(DBL_MEAS_BIT, Mixed, Int, Expected, errno);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    double  NextFrac, NextInt, NextMixed;
#define	NEXT_DBL_VECTOR(Mixed, Int, Frac, Err)			\
    {								\
	Write2Vector(DBL_MEAS_BIT, Mixed, Int, Frac, Err);	\
	_fpreset();						\
    }
    NextFrac = MyModf(NextMixed = pow(2,51) + .5, &NextInt);
    NEXT_DBL_VECTOR(NextMixed, NextInt, NextFrac, __NO_ERROR_)

    NEXT_DBL_VECTOR(DBL_MIN, 0.0, DBL_MIN, __NO_ERROR_);
    NEXT_DBL_VECTOR(DBL_MAX, DBL_MAX, 0.0, __NO_ERROR_);
    NEXT_DBL_VECTOR(nan(""), nan(""), 0.0, __NO_ERROR_);
    NEXT_DBL_VECTOR(infinity(), infinity(), 0.0, __NO_ERROR_);
    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"dddp\");}\n", Name, Name, Name, Name);
}
/* ----------------------------------------------------------------------- */
/* MyModff - Returns Fraction part of float Mixed No., stores Integer Part */
/* ----------------------------------------------------------------------- */
float
MyModff(float Mixed, float *Int)
{
    if (Mixed >= 0)
    {
	*Int = floorf(Mixed);
    }
    else
    {
	*Int = ceilf(Mixed);
    }
    return copysignf(Mixed - *Int, Mixed);
}
// ------------------------------------------------------------------
// GenFltVector - Generates Test Vectors for Float Precision Function
// ------------------------------------------------------------------
void
GenFltVector(char *Name)
{
    unsigned J;

    volatile  float Expected;

    // Redirect stdout to disk file

    AssignOutputFile(Name);

    // Generate first two lines of file

    GEN_FIRST_TWO_LINES

    // Generate vectors

    for (J = 0; J < (sizeof(CtlsX) / sizeof(CtlsX[0])); ++J)
    {
	double	Next;

	for (Next = CtlsX[J].Start; Next <= CtlsX[J].Stop;
		Next += CtlsX[J].Step)
	{
	    float  Frac, Int, Mixed = Next;

	    Frac = MyModff(Mixed, &Int);

	    if (Frac <= FLT_MAX)
	    {
		errno = 0;
		Expected = Frac;
	    }
	    else
	    {
		errno = ERANGE;
		Expected = infinityf();
		_fpreset();
	    }
	    Write2Vector(FLT_MEAS_BIT, Mixed, Int, Expected, errno);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    float   NextFrac, NextInt, NextMixed;
#define	NEXT_FLT_VECTOR(Mixed, Int, Frac, Err)			\
    {								\
	Write2Vector(FLT_MEAS_BIT, Mixed, Int, Frac, Err);	\
	_fpreset();						\
    }

    NextFrac = MyModff(NextMixed = powf(2,22) + .5, &NextInt);
    NEXT_FLT_VECTOR(NextMixed, NextInt, NextFrac, __NO_ERROR_)

    NEXT_FLT_VECTOR(FLT_MIN, 0.0, FLT_MIN, __NO_ERROR_);
    NEXT_FLT_VECTOR(FLT_MAX, FLT_MAX, 0.0, __NO_ERROR_);
    NEXT_FLT_VECTOR(nan(""), nan(""), 0.0, __NO_ERROR_);
    NEXT_FLT_VECTOR(infinityf(), infinityf(), 0.0, __NO_ERROR_);

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"fffp\");}\n", Name, Name, Name, Name);
}
int
main()
{
    signal(SIGINT, exit);
    GenDblVector("modf");
    GenFltVector("modff");
    exit(0);
}
