// ----------------------------------------------------------------
// genmiscv - generates test vectors for ceif[f], fabs[f], floor[f]
// ----------------------------------------------------------------
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

static	const	double	EPS = 1./131072.;

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

static	const char   *StrCeil   = "ceil";
static	const char   *StrCeilf  = "ceilf";
static	const char   *StrFabs   = "fabs";
static	const char   *StrFabsf  = "fabsf";
static	const char   *StrFloor  = "floor";
static	const char   *StrFloorf = "floorf";

inline qfloat xceil (const qfloat &x)
{
  qfloat y;
  qfloor (&x, &y);
  if (y < x) y += 1;
  return y;
}

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
	    if (EQUAL(Name, StrCeil))
	    {
		Expected = xtod(xceil(Argument));
		if (Expected == 0 && Argument < 0)
		{
		    __ieee_double_shape_type  MinusZero;
		    MinusZero.value = 0.0;
		    MinusZero.number.sign = 1;
		    Expected = MinusZero.value;
		}
	    }
	    else if (EQUAL(Name, StrFabs))
	    {
		Expected = xtod(xabs(Argument));
	    }
	    else if (EQUAL(Name, StrFloor))
	    {
		Expected = xtod(xfloor(Argument));
	    }
	    else
	    {
		fprintf(stderr, "Unknown Command String: %s\n", Name);
		break;
	    }
	    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
	if (K == Ctls[J].Start)
	    break;			/* Unknown Command String */
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = 0.0;
    if (EQUAL(Name, StrCeil))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFabs))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFloor))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    WriteVector(DBL_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    Argument = infinity();
    if (EQUAL(Name, StrCeil))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFabs))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFloor))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    WriteVector(DBL_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    Argument = -infinity();
    if (EQUAL(Name, StrCeil))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFabs))
    {
	Expected = -Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFloor))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    WriteVector(DBL_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    Argument = nan("");
    if (EQUAL(Name, StrCeil))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFabs))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFloor))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    WriteVector(DBL_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    Argument = -nan("");
    if (EQUAL(Name, StrCeil))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFabs))
    {
	Expected = -Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFloor))
    {
	Expected = Argument;
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
	    if (EQUAL(Name, StrCeilf))
	    {
		Expected = xtod(xceil(Argument));
		if (Expected == 0 && Argument < 0)
		{
		    __ieee_float_shape_type  MinusZero;
		    MinusZero.value = 0.0;
		    MinusZero.number.sign = 1;
		    Expected = MinusZero.value;
		}
	    }
	    else if (EQUAL(Name, StrFabsf))
	    {
		Expected = xtod(xabs(Argument));
	    }
	    else if (EQUAL(Name, StrFloorf))
	    {
		Expected = xtod(xfloor(Argument));
	    }
	    else
	    {
		fprintf(stderr, "Unknown Command String: %s\n", Name);
		break;
	    }
	    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
	if (K == Ctls[J].Start)
	    break;			/* Unknown Command String */
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = 0.0;
    if (EQUAL(Name, StrCeilf))
    {
	Expected = 0.0;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFabsf))
    {
	Expected = 0.0;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFloorf))
    {
	Expected = 0.0;
	errno    = __NO_ERROR_;
    }
    WriteVector(FLT_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    Argument = infinityf();
    if (EQUAL(Name, StrCeilf))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFabsf))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFloorf))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    WriteVector(FLT_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    Argument = -infinityf();
    if (EQUAL(Name, StrCeilf))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFabsf))
    {
	Expected = -Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFloorf))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    WriteVector(FLT_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    Argument = nan("");
    if (EQUAL(Name, StrCeilf))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFabsf))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFloorf))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    WriteVector(FLT_MEAS_BIT, Argument, Expected, errno);
    _fpreset();

    Argument = -nan("");
    if (EQUAL(Name, StrCeilf))
    {
	Expected = Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFabsf))
    {
	Expected = -Argument;
	errno    = __NO_ERROR_;
    }
    else if (EQUAL(Name, StrFloorf))
    {
	Expected = Argument;
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

    GenDblVector(StrCeil  );
    GenFltVector(StrCeilf );
    GenDblVector(StrFabs  );
    GenFltVector(StrFabsf );
    GenDblVector(StrFloor );
    GenFltVector(StrFloorf);

    exit(0);
}
