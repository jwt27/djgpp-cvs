# undef	_USE_LIBM_MATH_H
#include "test.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <float.h>
#include <math.h>

double
o_threshold=  7.09782712893383973096e+02,  /* 0x40862E42, 0xFEFA39EF */
u_threshold= -7.45133219101941108420e+02,  /* 0xc0874910, 0xD52D3051 */
ln2HI[2]   ={ 6.93147180369123816490e-01,  /* 0x3fe62e42, 0xfee00000 */
	     -6.93147180369123816490e-01,},/* 0xbfe62e42, 0xfee00000 */
ln2LO[2]   ={ 1.90821492927058770002e-10,  /* 0x3dea39ef, 0x35793c76 */
	     -1.90821492927058770002e-10,},/* 0xbdea39ef, 0x35793c76 */
invln2 =      1.44269504088896338700e+00;  /* 0x3ff71547, 0x652b82fe */

					   /* 3fe62e42 fee00000 */
					   /* 3dea39ef 35793c76 */
int
main()
{
	__ieee_double_shape_type  AnyDbl,
				  Ln2Hi, Ln2Lo, LibInvLn2, ActInvLn2;
	volatile __ieee_float_shape_type   AnyFlt;

	volatile float x = 13176794/4194304.;
	printf("Float(13176794.0/4194304.0) = %.21f\n", (float)x);
	x = powf(3,15);
	printf("+powf(3,15) = %+.16e\n", (float)x);
	x = -powf(3,15);
	printf("-powf(3,15) = %+.16e\n", (float)x);
	AnyFlt.value = 8.8722839111e+01;
	printf("(float)8.8722839111e+01 = 0x%08lx\n", AnyFlt.p1);

	AnyFlt.value = PI;
	AnyDbl.value = AnyFlt.value;
	printf("PI = %.19f, (%08lx %08lx)\n", PI,
		AnyDbl.parts.msw, AnyDbl.parts.lsw);

	Ln2Hi.value = .69314718036912381649017333984375;
	Ln2Lo.value = 1.908214929270587816144265680755e-10;

	printf("Correct Ln2Hi = %36.32f\t = %08lx %08lx\n",
		Ln2Hi.value, Ln2Hi.parts.msw, Ln2Hi.parts.lsw);
	printf("Correct Ln2Lo = %36.30e\t = %08lx %08lx\n\n",
		Ln2Lo.value, Ln2Lo.parts.msw, Ln2Lo.parts.lsw);

	Ln2Hi.value = ln2HI[0];
	Ln2Lo.value = ln2LO[0];

	printf("Library Ln2Hi = %36.32f\t = %08lx %08lx\n",
		Ln2Hi.value, Ln2Hi.parts.msw, Ln2Hi.parts.lsw);
	printf("Library Ln2Lo = %36.30e\t = %08lx %08lx\n\n",
		Ln2Lo.value, Ln2Lo.parts.msw, Ln2Lo.parts.lsw);

	LibInvLn2.value = invln2;
	ActInvLn2.value = 1.442695040888963407359924681;

	printf("Actual InvLn2 = %+33.30f\t = %08lx %08lx\n",
		ActInvLn2.value, ActInvLn2.parts.msw, ActInvLn2.parts.lsw);
	printf("Stated InvLn2 = %+33.30f\t = %08lx %08lx\n\n",
		LibInvLn2.value, LibInvLn2.parts.msw, LibInvLn2.parts.lsw);
	printf("log10(DBL_MIN)  = %.15e, log10(DBL_MAX) = %.15e\n",
		log10(DBL_MIN), log10(DBL_MAX));
	printf("log10(FLT_MIN)  = %.15e, log10(FLT_MAX) = %.15e\n",
		log10(FLT_MIN), log10(FLT_MAX));
	AnyFlt.value = sqrtf(FLT_MAX);
	printf("sqrtf(FLT_MAX)  = %.11e = 0x%08lx\n",
				AnyFlt.value, AnyFlt.p1);
	exit(0);
}
# if 0
Correct Ln2Hi =   0.69314718036912381649017333984375	 = 3fe62e42 fee00000
Correct Ln2Lo = 1.908214929270587700057409952237e-10	 = 3dea39ef 35793c76

Library Ln2Hi =   0.69314718036912381649017333984375	 = 3fe62e42 fee00000
Library Ln2Lo = 1.908214929270587700057409952237e-10	 = 3dea39ef 35793c76

Actual InvLn2 = +1.442695040888963387004650940071	 = 3ff71547 652b82fe
Stated InvLn2 = +1.442695040888963387004650940071	 = 3ff71547 652b82fe
# endif

