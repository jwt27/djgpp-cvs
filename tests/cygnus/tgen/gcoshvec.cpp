// ------------------------------------------------------
// gencoshv - generates test vectors for cosh() & coshf()
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
    {	/* Start   Step    Stop  */
	{ -10.0,  +0.25,   +10.0 },
	{ +11.0,  +1.0,    +18.0 },
	{ +19.0, 1./256., +19.0625 },
	{ +20.0,  +5.0,    +50.0 },
	{ +50.0,  +10.0,   +100  },
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
	    Expected = xtod(xcosh(Argument));

	    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = xtod(xlog(DBL_MAX));
    _fpreset();
    Expected = xtod(xcosh(Argument));

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);
    _fpreset();

    Argument = 1.0 + DBL_EPSILON;
    _fpreset();
    Expected = xtod(xcosh(Argument));

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);
    _fpreset();
    Argument = -infinity();
    Expected =  infinity();
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, ERANGE);
    _fpreset();

    Argument = +infinity();
    Expected = +infinity();
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, ERANGE);
    _fpreset();

    Argument = nan();
    Expected = nan();
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);
    _fpreset();

    Argument = -nan();
    Expected = -nan();
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);
    _fpreset();

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
	    Expected = xtod(xcosh(Argument));
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

    Argument = xtod(xacosh(FLT_MAX));
    Expected = xtod(xcosh(Argument));
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);
    _fpreset();

    Argument = 1.0+FLT_EPSILON;
    Expected = xtod(xcosh(Argument));
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);
    _fpreset();

    Argument = -infinityf();
    Expected =  infinityf();
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, ERANGE);
    _fpreset();

    Argument = +infinityf();
    Expected = +infinityf();
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, ERANGE);
    _fpreset();

    Argument = nan();
    Expected = nan();
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);
    _fpreset();

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
    GenDblVector("cosh");
    GenFltVector("coshf");
    exit(0);
}
