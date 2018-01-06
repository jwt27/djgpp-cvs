// --------------------------------------------------------
// genldxpv - generates test vectors for ldexp() & ldexpf()
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
	{  3.0,    2.0,     23.0, },
    };

LOOP_LIMITS  CtlsY[] =
    {	/* Start                   Step                             Stop  */
	{ -10.0,   1.0,   10.0, },
    };

/* INDENT ON */

#define	DBL_MEAS_BIT	62
#define	FLT_MEAS_BIT	34

// -------------------------------------------------------------------
// GenDblVector - Generates Test Vectors for Double Precision Function
// -------------------------------------------------------------------
void
GenDblVector(const char *Name)
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
		    volatile double  FracArgX = 1.0/ArgX;
		    qfloat  Q_Expect = xldexp(FracArgX, (long)ArgY);

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
	    	    Write2Vector(DBL_MEAS_BIT, FracArgX, ArgY, Expected,
			errno);
		}
	    }
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    ArgX = -0.0;
    ArgY = 1024;
    Expected = -0.0;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -1.0;
    ArgY = -2047;
    Expected = -0.0;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, ERANGE);

    ArgX =  1.0;
    ArgY = -2047;
    Expected = 0.0;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, ERANGE);

    ArgX = -1.0;
    ArgY = 2047;
    Expected = -infinity();
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, ERANGE);

    ArgX =  1.0;
    ArgY = 2047;
    Expected = infinity();
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, ERANGE);

    ArgX = infinity();
    ArgY = 2047;
    Expected = infinity();
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -infinity();
    ArgY = -2047;
    Expected = -infinity();
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = infinity();
    ArgY = 0;
    Expected = infinity();
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -infinity();
    ArgY = -0;
    Expected = -infinity();
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = nan("");
    ArgY = 2047;
    Expected = nan("");
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -nan("");
    ArgY = -2047;
    Expected = -nan("");
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"ddi\");}\n", Name, Name, Name, Name);
}
// ------------------------------------------------------------------
// GenFltVector - Generates Test Vectors for Float Precision Function
// ------------------------------------------------------------------
void
GenFltVector(const char *Name)
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
		    volatile float  FracArgX = 1.0/ArgX;
		    qfloat  Q_Expect = xldexp(FracArgX, (long)ArgY);

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
	    	    Write2Vector(FLT_MEAS_BIT, FracArgX, ArgY, Expected,
			errno);
		}
	    }
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    ArgX = -0.0;
    ArgY = -128;
    Expected = -0.0;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -1.0;
    ArgY = -500;
    Expected = -0.0;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, ERANGE);

    ArgX =  1.0;
    ArgY = -500;
    Expected = 0.0;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, ERANGE);

    ArgX = -1.0;
    ArgY = 128;
    Expected = -infinityf();
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, ERANGE);

    ArgX =  1.0;
    ArgY = 128;
    Expected = infinityf();
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, ERANGE);

    ArgX = infinityf();
    ArgY = 2047;
    Expected = infinityf();
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -infinityf();
    ArgY = -2047;
    Expected = -infinityf();
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = infinityf();
    ArgY = 0;
    Expected = infinityf();
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -infinityf();
    ArgY = -0;
    Expected = -infinityf();
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = nan("");
    ArgY = 2047;
    Expected = nan("");
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -nan("");
    ArgY = -2047;
    Expected = -nan("");
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"ffi\");}\n", Name, Name, Name, Name);
}
int
main()
{
    signal(SIGINT, exit);
    GenDblVector("ldexp");
    GenFltVector("ldexpf");
    exit(0);
}
