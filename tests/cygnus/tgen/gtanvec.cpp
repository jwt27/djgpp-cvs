// ---------------------------------------------------
// gentanv - generates test vectors for tan() & tanf()
// ---------------------------------------------------
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

// This is approximately PI to a small number of exact figures

#define	A_PI  (205887.0/65536.0)	//(13176794.0/4194304.0)

LOOP_LIMITS  Ctls[] =
    {	/* Start    Step	 Stop  */
	{ -A_PI,   +A_PI/64.,   +A_PI },
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
	    Expected = xtod(xtan(Argument));

	    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = 1e20;
    Expected = xtod(xtan(Argument));

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -infinity();
    Expected =  -nan();
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = +infinity();
    Expected = -nan();
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

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"dd\");}\n", Name, Name, Name, Name);
}
// -------------------------------------------------------------------
// GenFltVector - Generates Test Vectors for Float Precision Function
// -------------------------------------------------------------------
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
	    Expected = xtod(xtan(Argument));
	    if (Expected > FLT_MAX)
	    {
		errno = ERANGE;
	    }
	    else
	    {
		errno = __NO_ERROR_;
	    }
	    WriteVector(FLT_MEAS_BIT, Argument, Expected, errno);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = 1e20;
    Expected = xtod(xtan(Argument));

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -infinityf();
    Expected =  -nan();
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = +infinityf();
    Expected = -nan();
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = nan();
    Expected = nan();
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -nan();
    Expected = -nan();
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);
    _fpreset();

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"ff\");}\n", Name, Name, Name, Name);
}
int
main()
{
    signal(SIGINT, exit);
    GenDblVector("tan");
    GenFltVector("tanf");
    exit(0);
}
