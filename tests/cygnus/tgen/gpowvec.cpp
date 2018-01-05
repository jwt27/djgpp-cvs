// --------------------------------------------------------
// genpowv - generates test vectors for pow() & powf()
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
	{  1./8., (79./64.), 10.0 },
    };

LOOP_LIMITS  DblCtlsY[] =
    {	/* Start                   Step                             Stop  */
	{ log10(DBL_MIN), (log10(DBL_MAX)-log10(DBL_MIN))/32., log10(DBL_MAX) },
    };

LOOP_LIMITS  FltCtlsY[] =
    {	/* Start                   Step                             Stop  */
	{ log10(FLT_MIN), (log10(FLT_MAX)-log10(FLT_MIN))/32., log10(FLT_MAX) },
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
	    for (K = 0; K < (sizeof(DblCtlsY)/sizeof(DblCtlsY[0])); ++K)
	    {
		for (ArgY = DblCtlsY[K].Start; ArgY <= DblCtlsY[K].Stop;
			ArgY += DblCtlsY[K].Step)
		{
		    qfloat  Q_Expect = xpow(ArgX, ArgY);

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

    ArgX = -0.0;
    ArgY = 1.0;
    Expected = -0.0;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -0.0;
    ArgY = -1.0;
    Expected = -infinity();
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, EDOM);

    ArgX =  1.0;
    ArgY = -0.0;
    Expected = 1.0;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -1.0;
    ArgY = -0.0;
    Expected = 1.0;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -1.0;
    ArgY = infinity();
    Expected = -nan("");
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -2.0;
    ArgY = infinity();
    Expected = infinity();
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -1.0;
    ArgY = -infinity();
    Expected = -nan("");
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -0.5;
    ArgY = -infinity();
    Expected = infinity();
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -0.5;
    ArgY = +infinity();
    Expected = 0.0;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -infinity();
    ArgY = -infinity();
    Expected = 0.0;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -infinity();
    ArgY = +infinity();
    Expected = infinity();
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = +infinity();
    ArgY = +infinity();
    Expected = +infinity();
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = +infinity();
    ArgY = -infinity();
    Expected = 0.0;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = 1.0;
    ArgY = 1e20;
    Expected = 1.0;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -1.0;
    ArgY = -1e20;
    Expected = 1;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = 1.0;
    ArgY = nan("");
    Expected = nan("");
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = nan("");
    ArgY = infinity();
    Expected = nan("");
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = 10.1;
    ArgY = 400.1;
    Expected = infinity();
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, ERANGE);

    ArgX = 10.1;
    ArgY = -400.1;
    Expected = 0.0;
    Write2Vector(DBL_MEAS_BIT, ArgX, ArgY, Expected, ERANGE);

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
	    for (K = 0; K < (sizeof(DblCtlsY)/sizeof(DblCtlsY[0])); ++K)
	    {
		for (ArgY = FltCtlsY[K].Start; ArgY <= FltCtlsY[K].Stop;
			ArgY += FltCtlsY[K].Step)
		{
		    qfloat  Q_Expect = xpow(ArgX, ArgY);

		    if (Q_Expect <= (qfloat)FLT_MAX)
		    {
			errno = 0;
			Expected = xtod(Q_Expect);
		    }
		    else
		    {
			errno = ERANGE;
			Expected = infinityf();
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

    ArgX = -0.0;
    ArgY = 1.0;
    Expected = -0.0;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -0.0;
    ArgY = -1.0;
    Expected = -infinityf();
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, EDOM);

    ArgX =  1.0;
    ArgY = -0.0;
    Expected = 1.0;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -1.0;
    ArgY = -0.0;
    Expected = 1.0;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -1.0;
    ArgY = infinityf();
    Expected = -nan("");
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -2.0;
    ArgY = infinityf();
    Expected = infinityf();
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -1.0;
    ArgY = -infinityf();
    Expected = -nan("");
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -0.5;
    ArgY = -infinityf();
    Expected = infinityf();
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -0.5;
    ArgY = +infinityf();
    Expected = 0.0;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -infinityf();
    ArgY = -infinityf();
    Expected = 0.0;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -infinityf();
    ArgY = +infinityf();
    Expected = infinityf();
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = +infinityf();
    ArgY = +infinityf();
    Expected = +infinityf();
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = +infinityf();
    ArgY = -infinityf();
    Expected = 0.0;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = 1.0;
    ArgY = 1e20;
    Expected = 1.0;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = -1.0;
    ArgY = -1e20;
    Expected = 1;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = 1.0;
    ArgY = nan("");
    Expected = nan("");
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = nan("");
    ArgY = infinityf();
    Expected = nan("");
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, __NO_ERROR_);

    ArgX = 10.1;
    ArgY = 400.1;
    Expected = infinityf();
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, ERANGE);

    ArgX = 10.1;
    ArgY = -400.1;
    Expected = 0.0;
    Write2Vector(FLT_MEAS_BIT, ArgX, ArgY, Expected, ERANGE);

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"fff\");}\n", Name, Name, Name, Name);
}
int
main()
{
    signal(SIGINT, exit);
    GenDblVector("pow");
    GenFltVector("powf");
    exit(0);
}
