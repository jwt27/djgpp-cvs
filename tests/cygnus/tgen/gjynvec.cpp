// ---------------------------------------------------
// genjy01v - generates test vectors for jn,jnf,yn,ynf
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
#define USE_WRITE_2_VECTOR
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

static	char   *StrJn	= "jn";
static	char   *StrJnf	= "jnf";
static	char   *StrYn	= "yn";
static	char   *StrYnf	= "ynf";
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
	    for (int N = 2; N <= 6; ++N)
	    {
	    	if (EQUAL(Name, StrJn))
	    	{
		    Expected = xtod(xjn(N, Argument));
	    	}
	    	else if (EQUAL(Name, StrYn))
	    	{
		    Expected = xtod(xyn(N, Argument));
	    	}
	    	Write2Vector(DBL_MEAS_BIT, N, Argument, Expected,
			__NO_ERROR_);
	    }
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    for (int N = 2; N <= 6; ++N)
    {
    	Argument = 0.0;
    	if (EQUAL(Name, StrJn))
    	{
	    Expected = 0.0;
	    errno    = __NO_ERROR_;
    	}
    	else
    	{
	    Expected = -infinity();
	    errno	 = EDOM;
    	}

    	Write2Vector(DBL_MEAS_BIT, N, Argument, Expected, errno);
    	_fpreset();
    }

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"did\");}\n", Name, Name, Name, Name);
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
	    for (int N = 2; N <= 6; ++N)
	    {
	    	if (EQUAL(Name, StrJnf))
	    	{
		    Expected = xtod(xjn(N, Argument));
	    	}
	    	else if (EQUAL(Name, StrYnf))
	    	{
		    Expected = xtod(xyn(N, Argument));
	    	}
	    	Write2Vector(FLT_MEAS_BIT, N, Argument, Expected,
			__NO_ERROR_);
	    }
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    for (int N = 2; N <= 6; ++N)
    {
    	Argument = 0.0;
    	if (EQUAL(Name, StrJnf))
    	{
	    Expected = 0.0;
	    errno    = __NO_ERROR_;
    	}
    	else
    	{
	    Expected = -infinityf();
	    errno	 = EDOM;
    	}
    	Write2Vector(FLT_MEAS_BIT, N, Argument, Expected, errno);
    	_fpreset();
    }

    // Generate last lines (code lines) of file

    printf("0,};\nvoid\ntest_%s(int m)\t{ run_vector_1(m, %s_vec,(char *)"
	"(%s),\"%s\",\"fif\");}\n", Name, Name, Name, Name);
}
int
main()
{
    signal(SIGINT, exit);

    GenDblVector(StrJn);
    GenFltVector(StrJnf);
    GenDblVector(StrYn);
    GenFltVector(StrYnf);

    exit(0);
}
