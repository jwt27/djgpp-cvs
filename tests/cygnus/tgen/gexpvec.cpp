// ---------------------------------------------------
// genexpv - generates test vectors for exp() & expf()
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

LOOP_LIMITS  DblCtls[] =
    {	/* Start   Step    Stop  */
	{ -log(2)/2.,  log(2)/64., log(2)/2. },
	{ log(DBL_MIN), log(DBL_MAX)/64., log(DBL_MAX) },
    };

LOOP_LIMITS  FltCtls[] =
    {	/* Start   Step    Stop  */
	{ -log(2)/2.,  log(2)/64., log(2)/2. },
	{ log(FLT_MIN), log(FLT_MAX)/64., log(FLT_MAX) },
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

    for (J = 0; J < (sizeof(DblCtls) / sizeof(DblCtls[0])); ++J)
    {
	for (K = DblCtls[J].Start; K <= DblCtls[J].Stop; K += DblCtls[J].Step)
	{
	    Argument = K;
	    Expected = xtod(xexp(Argument));

	    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);
	}
    }

    // --------------
    // SPECIAL VALUES
    // --------------

    Argument = 1e20;
    Expected = infinity();
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, ERANGE);

    Argument = log(DBL_MAX);
    Expected = xtod(xexp(Argument));
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = log(DBL_MIN);
    Expected = xtod(xexp(Argument));
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = log(pow(2,-1074.99999999999));
    Expected = xtod(xexp(Argument));
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -infinity();
    Expected =  0;
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = +infinity();
    Expected = +infinity();
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = nan("");
    Expected = nan("");
    _fpreset();

    WriteVector(DBL_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -nan("");
    Expected = -nan("");
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

    for (J = 0; J < (sizeof(FltCtls) / sizeof(FltCtls[0])); ++J)
    {
	for (K = FltCtls[J].Start; K <= FltCtls[J].Stop; K += FltCtls[J].Step)
	{
	    Argument = K;
	    Expected = xtod(xexp(Argument));
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
    Expected = infinityf();
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, ERANGE);

    Argument = logf(FLT_MAX);
    Expected = xtof(xexp(Argument));
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = logf(FLT_MIN);
    Expected = xtof(xexp(Argument));
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = log(pow(2,-149.9999));
    Expected = xtod(xexp(Argument));
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -infinityf();
    Expected =  0;
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = +infinityf();
    Expected = +infinityf();
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = nan("");
    Expected = nan("");
    _fpreset();

    WriteVector(FLT_MEAS_BIT, Argument, Expected, __NO_ERROR_);

    Argument = -nan("");
    Expected = -nan("");
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
    GenDblVector("exp");
    GenFltVector("expf");
    exit(0);
}
