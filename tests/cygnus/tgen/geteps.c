#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"
int
main()
{
    unsigned k, m;
    double  Choice[] = {0.5, 0.975, 1.0};
    __ieee_double_shape_type Arg;

    printf("DBL_EPSILON   = %.25e\n", DBL_EPSILON);
    printf("1/DBL_EPSILON = %f\n", 1.0 / DBL_EPSILON);
    printf("FLT_EPSILON   = %.15e\n", FLT_EPSILON);
    printf("1/FLT_EPSILON = %f\n", 1.0 / FLT_EPSILON);

    printf("\n\tCALCULATING PLUS DOUBLE EPS NUMBERS\n");
    for (k = 0; k < sizeof(Choice) / sizeof(Choice[0]); ++k)
    {
	__ieee_double_shape_type Arg;
	volatile double Base, Denom, Eps, EpsLast, EpsNeg, Last;
	Base = Choice[k];
	EpsLast = 0;
	m = 0;
	for (Eps = 0.5; (Last = Base + Eps) > Base; Eps /= 2.0)
	{
	    ++m;
	    EpsLast = Eps;
	}
	Denom = 1 / EpsLast;
	printf("For Base = %f, Eps = %.25e (1/2^%u)\n",
	    Base, EpsLast, m);

	Arg.value = Base + EpsLast;
	printf("Base + EpsLast = %.25e = %08lx %08lx\n",
	    Arg.value, Arg.parts.msw, Arg.parts.lsw);
    }
    printf("\n\tCALCULATING MINUS DOUBLE EPS NUMBERS\n");
    for (k = 0; k < sizeof(Choice) / sizeof(Choice[0]); ++k)
    {
	__ieee_double_shape_type Arg;
	volatile double Base, Denom, Eps, EpsLast, EpsNeg, Last;
	Base = Choice[k];
	EpsLast = 0;
	m = 0;
	for (EpsNeg = 0.5; (Last = Base - EpsNeg) < Base; EpsNeg /= 2.0)
	{
	    ++m;
	    EpsLast = EpsNeg;
	}
	Denom = 1 / EpsLast;
	printf("For Base = %f, EpsNeg = %.25e (1/2^%u)\n",
	    Base, EpsLast, m);

	Arg.value = Base - EpsLast;
	printf("Base - EpsLast = %.25e = %08lx %08lx\n",
	    Arg.value, Arg.parts.msw, Arg.parts.lsw);
    }
    printf("\n\tCALCULATING PLUS FLOAT EPS NUMBERS\n");
    for (k = 0; k < sizeof(Choice) / sizeof(Choice[0]); ++k)
    {
	__ieee_float_shape_type Arg;
	volatile float Base, Denom, Eps, EpsLast, EpsNeg, Last;
	Base = Choice[k];
	EpsLast = 0;
	m = 0;
	for (Eps = 0.5; (Last = Base + Eps) > Base; Eps /= 2.0)
	{
	    ++m;
	    EpsLast = Eps;
	}
	Denom = 1 / EpsLast;
	printf("For Base = %f, Eps = %.15e (1/2^%u)\n",
	    Base, EpsLast, m);

	Arg.value = Base + EpsLast;
	printf("Base + EpsLast = %.15e = %08lx\n",
	    Arg.value, Arg.p1);
    }
    printf("\n\tCALCULATING MINUS FLOAT EPS NUMBERS\n");
    for (k = 0; k < sizeof(Choice) / sizeof(Choice[0]); ++k)
    {
	__ieee_float_shape_type Arg;
	volatile float Base, Denom, Eps, EpsLast, EpsNeg, Last;
	Base = Choice[k];
	EpsLast = 0;
	m = 0;
	for (EpsNeg = 0.5; (Last = Base - EpsNeg) < Base; EpsNeg /= 2.0)
	{
	    ++m;
	    EpsLast = EpsNeg;
	}
	Denom = 1 / EpsLast;
	printf("For Base = %f, EpsNeg = %.15e (1/2^%u)\n",
	    Base, EpsLast, m);

	Arg.value = Base - EpsLast;
	printf("Base - EpsLast = %.15e = %08lx\n",
	    Arg.value, Arg.p1);
    }
    exit(0);
}
