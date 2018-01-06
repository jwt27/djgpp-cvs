// --------------------------------------------------------
// genacshv - generates test vectors for acosh() & acoshf()
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
#include "genmathv.h"

/* INDENT OFF */

LOOP_LIMITS  Ctls[] =
    {	/* Start   Step      Stop  */
	{ +1.0,  +0.015625,   2.0 },
	{ +3.0,  +1.015625,  68.0 },
	{ +75.0, +25.0,     500.0 },
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
    unsigned J;
    double   K;

    volatile  double  Argument, Expected;

    // Redirect stdout to disk file

    AssignOutputFile(Name);

    // Generate first two lines of file

    GEN_FIRST_TWO_LINES

    // Generate vectors

    for (J = 0; J < (sizeof(Ctls) / sizeof(Ctls[0])); ++J)
    {
	for (K = Ctls[J].Start; K <= Ctls[J].Stop; K += Ctls[J].Step)
	{
	    Argument = K;
	    Expected = xtod(xacosh(Argument));

	    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = -1.125;
    Expected = nan("");
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, EDOM);

    Argument = 0;
    Expected = nan("");
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, EDOM);

    Argument = nan("");
    Expected = nan("");
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -infinity();
    Expected =  nan("");
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, EDOM);

    Argument = infinity();
    Expected = infinity();
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"dd\");}\n", Name, Name, Name, Name);
}
// -------------------------------------------------------------------
// GenFltVector - Generates Test Vectors for Float Precision Function
// -------------------------------------------------------------------
void
GenFltVector(const char *Name)
{
    unsigned J;
    float    K;

    volatile  float  Argument, Expected;

    // Redirect stdout to disk file

    AssignOutputFile(Name);

    // Generate first two lines of file

    GEN_FIRST_TWO_LINES

    // Generate vectors

    for (J = 0; J < (sizeof(Ctls) / sizeof(Ctls[0])); ++J)
    {
	for (K = Ctls[J].Start; K <= Ctls[J].Stop; K += Ctls[J].Step)
	{
	    Argument = K;
	    Expected = xtod(xacosh(Argument));

	    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = -1.125;
    Expected = nan("");
    _fpreset();
    WriteVector(FLT_MEAS_BIT, Argument, Expected, EDOM);

    Argument = 0;
    Expected = nan("");
    _fpreset();
    WriteVector(FLT_MEAS_BIT, Argument, Expected, EDOM);

    Argument = nan("");
    Expected = nan("");
    _fpreset();
    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -infinityf();
    Expected =  nan("");
    _fpreset();
    WriteVector(FLT_MEAS_BIT, Argument, Expected, EDOM);

    Argument = infinityf();
    Expected = infinityf();
    _fpreset();
    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"ff\");}\n", Name, Name, Name, Name);
}
int
main()
{
    signal(SIGINT, exit);
    GenDblVector("acosh");
    GenFltVector("acoshf");
    exit(0);
}
