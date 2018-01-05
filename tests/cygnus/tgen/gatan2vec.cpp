// --------------------------------------------------------
// genatn2v - generates test vectors for atan2() & atan2f()
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

    for (J = 2; J <= pow(2,16); J+=J)
    {
	ArgX = J;

	for (K = 1; K <= pow(3,15); K *= 3)
	{
	    ArgY = K;
	    Expected = xtod(xatan2(ArgY, ArgX));

	    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    ArgY = -0.6;
    ArgX = 1.2;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -0.6;
    ArgX = -1.2;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -0.0;
    ArgX = 1.0;
    Expected = -0.0;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -0.0;
    ArgX = -1.0;
    Expected = -xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY =  1.0;
    ArgX = -0.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -1.0;
    ArgX = -0.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -1.0;
    ArgX = infinity();
    Expected = -0.0;
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -1.0;
    ArgX = -infinity();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = +infinity();
    ArgX = -1.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -infinity();
    ArgX = -infinity();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -infinity();
    ArgX = +infinity();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = +infinity();
    ArgX = +infinity();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = +infinity();
    ArgX = -infinity();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = 1e20;
    ArgX = 1.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -1e20;
    ArgX = -1.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = 1e-20;
    ArgX = 1.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -1e-20;
    ArgX = 1.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = 1.0;
    ArgX = nan("");
    Expected = nan("");
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = nan("");
    ArgX = infinity();
    Expected = nan("");
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = infinity();
    ArgX = infinity();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -infinity();
    ArgX = infinity();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(DBL_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

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

    for (J = 2; J <= pow(2,16); J+=J)
    {
	ArgX = J;

	for (K = 1; K <= pow(3,15); K *= 3)
	{
	    ArgY = K;
	    Expected = xtod(xatan2(ArgY, ArgX));

	    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    ArgY = -0.6;
    ArgX = 1.2;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -0.6;
    ArgX = -1.2;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -0.0;
    ArgX = 1.0;
    Expected = -0.0;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -0.0;
    ArgX = -1.0;
    Expected = -xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY =  1.0;
    ArgX = -0.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -1.0;
    ArgX = -0.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -1.0;
    ArgX = infinityf();
    Expected = -0.0;
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -1.0;
    ArgX = -infinityf();
    Expected = -acosf(ArgY);
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = +infinityf();
    ArgX = -1.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -infinityf();
    ArgX = -infinityf();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -infinityf();
    ArgX = +infinityf();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = +infinityf();
    ArgX = +infinityf();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = +infinityf();
    ArgX = -infinityf();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = 1e20;
    ArgX = 1.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -1e20;
    ArgX = -1.0;
    Expected = asinf(ArgX);
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = 1e-20;
    ArgX = 1.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -1e-20;
    ArgX = 1.0;
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = 1.0;
    ArgX = nan("");
    Expected = nan("");
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = nan("");
    ArgX = infinityf();
    Expected = nan("");
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = infinityf();
    ArgX = infinityf();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    ArgY = -infinityf();
    ArgX = infinityf();
    Expected = xtod(xatan2(ArgY, ArgX));
    _fpreset();
    Write2Vector(FLT_MEAS_BIT, ArgY, ArgX, Expected, __NO_ERROR_);

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"fff\");}\n", Name, Name, Name, Name);
}
int
main()
{
    signal(SIGINT, exit);
    GenDblVector("atan2");
    GenFltVector("atan2f");
    exit(0);
}
