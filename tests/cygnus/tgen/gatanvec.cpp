// ------------------------------------------------------
// genatanv - generates test vectors for atan() & atanf()
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
#include "genmathv.h"

/* INDENT OFF */

LOOP_LIMITS  Ctls[] =
    {	/* Start   Step     Stop  */
	{   0.0,  2.5/128., 2.5,  },
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
	    Expected = xtod(xatan(Argument));

	    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = 1e20;
    Expected = xtod(xatan(Argument));
    _fpreset();
    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -1e20;;
    Expected = xtod(xatan(Argument));
    _fpreset();
    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = 1e-20;
    Expected = xtod(xatan(Argument));
    _fpreset();
    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -1e-20;;
    Expected = xtod(xatan(Argument));
    _fpreset();
    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = nan();
    Expected = nan();
    _fpreset();
    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -nan();
    Expected = -nan();
    _fpreset();
    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = infinity();
    Expected = xtod(xatan(Argument));
    _fpreset();
    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -infinity();
    Expected = xtod(xatan(Argument));
    _fpreset();
    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"dd\");}\n", Name, Name, Name, Name);
}
// ------------------------------------------------------------------
// GenFltVector - Generates Test Vectors for Float Precision Function
// ------------------------------------------------------------------
void
GenFltVector(char *Name)
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
	    Expected = xtof(xatan(Argument));

	    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = 1e20;
    Expected = xtod(xatan(Argument));
    _fpreset();
    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -1e20;;
    Expected = xtod(xatan(Argument));
    _fpreset();
    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = 1e-20;
    Expected = xtod(xatan(Argument));
    _fpreset();
    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -1e-20;;
    Expected = xtod(xatan(Argument));
    _fpreset();
    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = nan();
    Expected = nan();
    _fpreset();
    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -nan();
    Expected = -nan();
    _fpreset();
    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = infinityf();
    Expected = xtod(xatan(Argument));
    _fpreset();
    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -infinityf();
    Expected = xtod(xatan(Argument));
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
    GenDblVector("atan");
    GenFltVector("atanf");
    exit(0);
}
