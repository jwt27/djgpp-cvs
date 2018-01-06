// -----------------------------------------------------------
// generfv - generates test vectors for erf, erff, erfc, erfcf
// -----------------------------------------------------------
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
static	const	double	EPS = 1./8388608.;
LOOP_LIMITS  Ctls[] =
    {	/* Start           Step	 	Stop  */
	{ -7.0-EPS,    	   1./4,	-5.0-EPS,  },
	{ -4.0-EPS,        1./16.,   	 0.0-EPS, },
	{ 0.0+EPS,         1./16.,	 4.0+EPS,   },
	{ 5.0,     	   1./4,	 7.0,  },
    };

/* INDENT ON */

#define	DBL_MEAS_BIT	62
#define	FLT_MEAS_BIT	34

static	const char   *StrErf	 = "erf";
static	const char   *StrErff	 = "erff";
static	const char   *StrErfc	 = "erfc";
static	const char   *StrErfcf = "erfcf";
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
	    if (EQUAL(Name, StrErf))
	    {
		Expected = xtod(xerf(Argument));
	    }
	    else if (EQUAL(Name, StrErfc))
	    {
		Expected = xtod(xerfc(Argument));
	    }
	    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = 0.0;
    if (EQUAL(Name, StrErf))
    {
	Expected = 0.0;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrErfc))
    {
	Expected = 1.0;
	errno    = __NO_ERROR_;
    }
    WriteVector(DBL_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    Argument = infinity();
    if (EQUAL(Name, StrErf))
    {
	Expected = 1.0;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrErfc))
    {
	Expected = 0.0;
	errno    = __NO_ERROR_;
    }
    WriteVector(DBL_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    Argument = -infinity();
    if (EQUAL(Name, StrErf))
    {
	Expected = -1.0;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrErfc))
    {
	Expected = +2.0;
	errno    = __NO_ERROR_;
    }
    WriteVector(DBL_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

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
    double   K;

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
	    if (EQUAL(Name, StrErff))
	    {
		Expected = xtod(xerf(Argument));
	    }
	    else if (EQUAL(Name, StrErfcf))
	    {
		Expected = xtod(xerfc(Argument));
	    }
	    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = 0.0;
    if (EQUAL(Name, StrErff))
    {
	Expected = 0.0;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrErfcf))
    {
	Expected = 1.0;
	errno    = __NO_ERROR_;
    }
    WriteVector(FLT_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    Argument = infinity();
    if (EQUAL(Name, StrErff))
    {
	Expected = 1.0;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrErfcf))
    {
	Expected = 0.0;
	errno    = __NO_ERROR_;
    }
    WriteVector(FLT_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    Argument = -infinity();
    if (EQUAL(Name, StrErff))
    {
	Expected = -1.0;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrErfcf))
    {
	Expected = +2.0;
	errno    = __NO_ERROR_;
    }
    WriteVector(FLT_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"ff\");}\n", Name, Name, Name, Name);
}
int
main()
{
    signal(SIGINT, exit);

    GenDblVector(StrErf);
    GenFltVector(StrErff);
    GenDblVector(StrErfc);
    GenFltVector(StrErfcf);

    exit(0);
}
