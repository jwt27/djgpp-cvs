// ------------------------------------------------------
// genfmodv - generates test vectors for fmod() & fmodf()
// ------------------------------------------------------
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
    {	/* Start   Step    Stop  */
	{ -11.25,  0.625,  11.25, },
    };

LOOP_LIMITS  CtlsY[] =
    {	/* Start	Step       Stop  */
	{  -11.0,      0.625,      9.0,  },
    };


/* INDENT ON */

#define	DBL_MEAS_BIT	62
#define	FLT_MEAS_BIT	34
#define	XHYPOT(x, y)	(xsqrt(((qfloat)(x))*(x) + ((qfloat)(y))*(y)))
// -------------------------------------------------------------------
// GenDblVector - Generates Test Vectors for Double Precision Function
// -------------------------------------------------------------------
void
GenDblVector(char *Name)
{
    unsigned J, K;

    volatile  double  ArgX, ArgY, Expected;

    // Redirect stdout to disk file

    AssignOutputFile(Name);

    // Generate first two lines of file

    GEN_FIRST_TWO_LINES

    // Generate vectors

    for (J = 0; J < (sizeof(CtlsX) / sizeof(CtlsX[0])); ++J)
    {
	for (ArgX = CtlsX[J].Start; ArgX <= CtlsX[J].Stop;
		ArgX += CtlsX[J].Step)
	{
	    for (K = 0; K < (sizeof(CtlsY)/sizeof(CtlsY[0])); ++K)
	    {
		for (ArgY = CtlsY[K].Start; ArgY <= CtlsY[K].Stop;
			ArgY += CtlsY[K].Step)
		{
		    qfloat  Q_Expect = (qfloat)ArgX % (qfloat)ArgY;

		    if (Q_Expect <= (qfloat)DBL_MAX)
		    {
			errno = 0;
			Expected = xtod(Q_Expect);
		    }
		    else
		    {
			errno = ERANGE;
			Expected = infinity();
			_fpreset();
		    }
	    	    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected,
			errno);
		}
	    }
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    double  NextAns;
#define	NEXT_DBL_VECTOR(x, y, Ans, Err)				\
    {								\
	ArgX = x;						\
	ArgY = y;						\
	Expected = Ans;						\
	Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, Err);	\
	_fpreset();						\
    }
    NextAns = nan();
    _fpreset();
    NEXT_DBL_VECTOR(0, 0, NextAns, EDOM)

    NextAns = 0.0;
    NEXT_DBL_VECTOR(DBL_MIN, DBL_MIN, NextAns, __NO_ERROR_)

    NextAns = 0.0;
    NEXT_DBL_VECTOR(DBL_MAX, DBL_MAX, NextAns, __NO_ERROR_)

    NextAns = nan();
    _fpreset();
    NEXT_DBL_VECTOR(DBL_MAX, 0, NextAns, EDOM)

    NextAns = 0;
    NEXT_DBL_VECTOR(DBL_MAX, DBL_MIN, NextAns, __NO_ERROR_)

    NextAns = DBL_MIN;
    NEXT_DBL_VECTOR(DBL_MIN, DBL_MAX, NextAns, __NO_ERROR_)

    NextAns = -nan();
    _fpreset();
    NEXT_DBL_VECTOR(infinity(), infinity(), NextAns, __NO_ERROR_)

    NextAns = -nan();
    _fpreset();
    NEXT_DBL_VECTOR(-infinity(), -infinity(), NextAns, __NO_ERROR_)

    NextAns = nan();
    _fpreset();
    NEXT_DBL_VECTOR(nan(), nan(), NextAns, __NO_ERROR_)

    NextAns = nan();
    _fpreset();
    NEXT_DBL_VECTOR(infinity(), nan(), NextAns, __NO_ERROR_)

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"ddd\");}\n", Name, Name, Name, Name);
}
// ------------------------------------------------------------------
// GenFltVector - Generates Test Vectors for Float Precision Function
// ------------------------------------------------------------------
void
GenFltVector(char *Name)
{
    unsigned J, K;

    volatile  float  ArgX, ArgY, Expected;

    // Redirect stdout to disk file

    AssignOutputFile(Name);

    // Generate first two lines of file

    GEN_FIRST_TWO_LINES

    // Generate vectors

    for (J = 0; J < (sizeof(CtlsX) / sizeof(CtlsX[0])); ++J)
    {
	for (ArgX = CtlsX[J].Start; ArgX <= CtlsX[J].Stop;
		ArgX += CtlsX[J].Step)
	{
	    for (K = 0; K < (sizeof(CtlsY)/sizeof(CtlsY[0])); ++K)
	    {
		for (ArgY = CtlsY[K].Start; ArgY <= CtlsY[K].Stop;
			ArgY += CtlsY[K].Step)
		{
		    qfloat  Q_Expect = (qfloat)ArgX % (qfloat)ArgY;

		    if (Q_Expect <= (qfloat)FLT_MAX)
		    {
			errno = 0;
			Expected = xtod(Q_Expect);
		    }
		    else
		    {
			errno = ERANGE;
			Expected = infinity();
			_fpreset();
		    }
	    	    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected,
			errno);
		}
	    }
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    float   NextAns;
#define	NEXT_FLT_VECTOR(x, y, Ans, Err)				\
    {								\
	ArgX = x;						\
	ArgY = y;						\
	Expected = Ans;						\
	Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, Err);	\
	_fpreset();						\
    }
    NextAns = nan();
    _fpreset();
    NEXT_FLT_VECTOR(0, 0, NextAns, EDOM)

    NextAns = 0.0;
    NEXT_FLT_VECTOR(FLT_MIN, FLT_MIN, NextAns, __NO_ERROR_)

    NextAns = 0.0;
    NEXT_FLT_VECTOR(FLT_MAX, FLT_MAX, NextAns, __NO_ERROR_)

    NextAns = nan();
    _fpreset();
    NEXT_FLT_VECTOR(FLT_MAX, 0, NextAns, EDOM)

    NextAns = 0;
    NEXT_FLT_VECTOR(FLT_MAX, FLT_MIN, NextAns, __NO_ERROR_)

    NextAns = FLT_MIN;
    NEXT_FLT_VECTOR(FLT_MIN, FLT_MAX, NextAns, __NO_ERROR_)

    NextAns = -nan();
    _fpreset();
    NEXT_FLT_VECTOR(infinityf(), infinityf(), NextAns, __NO_ERROR_)

    NextAns = -nan();
    _fpreset();
    NEXT_FLT_VECTOR(-infinityf(), -infinityf(), NextAns, __NO_ERROR_)

    NextAns = nan();
    _fpreset();
    NEXT_FLT_VECTOR(nan(), nan(), NextAns, __NO_ERROR_)

    NextAns = nan();
    _fpreset();
    NEXT_FLT_VECTOR(infinity(), nan(), NextAns, __NO_ERROR_)

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"fff\");}\n", Name, Name, Name, Name);
}
int
main()
{
    signal(SIGINT, exit);
    GenDblVector("fmod");
    GenFltVector("fmodf");
    exit(0);
}
