// -----------------------------------------------------------------
// genjy01v - generates test vectors for j0,j0f,y0,y0f,j1,j1f,y1,y1f
// -----------------------------------------------------------------
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
    {	/* Start           Step	 	Stop  */
	{ 1./8388608.,     1./32.,	5.0,   },
	{ 10.0,     	   40./32.,	50.0,  },
	{ 60.25,           0.5,  	70.25, },
    };

/* INDENT ON */

#define	DBL_MEAS_BIT	62
#define	FLT_MEAS_BIT	34

static	char   *StrJ0	= "j0";
static	char   *StrJ0f	= "j0f";
static	char   *StrJ1	= "j1";
static	char   *StrJ1f	= "j1f";
static	char   *StrY0	= "y0";
static	char   *StrY0f	= "y0f";
static	char   *StrY1	= "y1";
static	char   *StrY1f	= "y1f";
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
	    if (EQUAL(Name, StrJ0))
	    {
		Expected = xtod(xjn(0, Argument));
	    }
	    else if (EQUAL(Name, StrJ1))
	    {
		Expected = xtod(xjn(1, Argument));
	    }
	    else if (EQUAL(Name, StrY0))
	    {
		Expected = xtod(xyn(0, Argument));
	    }
	    else if (EQUAL(Name, StrY1))
	    {
		Expected = xtod(xyn(1, Argument));
	    }
	    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = 0.0;
    if (EQUAL(Name, StrJ0))
    {
	Expected = 1.0;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrJ1))
    {
	Expected = 0.0;
	errno    = __NO_ERROR_;
    }
    else
    {
	Expected = -infinity();
	errno	 = EDOM;
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
GenFltVector(char *Name)
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
	    if (EQUAL(Name, StrJ0f))
	    {
		Expected = xtod(xjn(0, Argument));
	    }
	    else if (EQUAL(Name, StrJ1f))
	    {
		Expected = xtod(xjn(1, Argument));
	    }
	    else if (EQUAL(Name, StrY0f))
	    {
		Expected = xtod(xyn(0, Argument));
	    }
	    else if (EQUAL(Name, StrY1f))
	    {
		Expected = xtod(xyn(1, Argument));
	    }
	    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = 0.0;
    if (EQUAL(Name, StrJ0f))
    {
	Expected = 1.0;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrJ1f))
    {
	Expected = 0.0;
	errno    = __NO_ERROR_;
    }
    else
    {
	Expected = -infinityf();
	errno	 = EDOM;
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

    GenDblVector(StrJ0);
    GenFltVector(StrJ0f);
    GenDblVector(StrJ1);
    GenFltVector(StrJ1f);
    GenDblVector(StrY0);
    GenFltVector(StrY0f);
    GenDblVector(StrY1);
    GenFltVector(StrY1f);

    exit(0);
}
