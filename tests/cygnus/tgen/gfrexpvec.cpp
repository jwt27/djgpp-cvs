// --------------------------------------------------------
// genfrxpv - generates test vectors for frexp() & frexpf()
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
	    long    Int;
	    double  Frac;

	    Frac = xtod(xfrexp(Mixed, Int));

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

    long    NextInt = 0;
    double  NextFrac = 0, NextMixed = 0;
#define	NEXT_DBL_VECTOR(Mixed, Int, Frac, Err)			\
    {								\
	Write2Vector(DBL_MEAS_BIT, Mixed, Int, Frac, Err);	\
	_fpreset();						\
    }
    NEXT_DBL_VECTOR(NextMixed, NextInt, NextFrac, __NO_ERROR_)

    NextFrac = xtod(xfrexp(DBL_MIN, NextInt));
    NEXT_DBL_VECTOR(DBL_MIN, NextInt, NextFrac, __NO_ERROR_);

    NextFrac = xtod(xfrexp(DBL_MAX, NextInt));
    NEXT_DBL_VECTOR(DBL_MAX, NextInt, NextFrac, __NO_ERROR_);

    NEXT_DBL_VECTOR(nan(""), 0.0, nan(""), __NO_ERROR_);
    NEXT_DBL_VECTOR(infinity(), 0.0, infinity(), __NO_ERROR_);

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"ddip\");}\n", Name, Name, Name, Name);
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
	    long    Int;
	    float   Frac, Mixed = Next;

	    Frac = xtod(xfrexp(Mixed, Int));

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
    long    NextInt = 0;
    float   NextFrac = 0, NextMixed = 0;
#define	NEXT_FLT_VECTOR(Mixed, Int, Frac, Err)			\
    {								\
	Write2Vector(FLT_MEAS_BIT, Mixed, Int, Frac, Err);	\
	_fpreset();						\
    }
    NEXT_FLT_VECTOR(NextMixed, NextInt, NextFrac, __NO_ERROR_)

    NextFrac = xtod(xfrexp(FLT_MIN, NextInt));
    NEXT_FLT_VECTOR(FLT_MIN, NextInt, NextFrac, __NO_ERROR_);

    NextFrac = xtod(xfrexp(FLT_MAX, NextInt));
    NEXT_FLT_VECTOR(FLT_MAX, NextInt, NextFrac, __NO_ERROR_);

    NEXT_FLT_VECTOR(nan(""), 0.0, nan(""), __NO_ERROR_);
    NEXT_FLT_VECTOR(infinity(), 0.0, infinity(), __NO_ERROR_);

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"ffip\");}\n", Name, Name, Name, Name);
}
int
main()
{
    signal(SIGINT, exit);
    GenDblVector("frexp");
    GenFltVector("frexpf");
    exit(0);
}
