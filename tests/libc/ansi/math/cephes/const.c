/*							const.c
 *
 *	Globally declared constants
 *
 *
 *
 * SYNOPSIS:
 *
 * extern double nameofconstant;
 *
 *
 *
 *
 * DESCRIPTION:
 *
 * This file contains a number of mathematical constants and
 * also some needed size parameters of the computer arithmetic.
 * The values are supplied as arrays of hexadecimal integers
 * for IEEE arithmetic; arrays of octal constants for DEC
 * arithmetic; and in a normal decimal scientific notation for
 * other machines.  The particular notation used is determined
 * by a symbol (DEC, IBMPC, or UNK) defined in the include file
 * mconf.h.
 *
 * The default size parameters are as follows.
 *
 * For DEC and UNK modes:
 * MACHEP =  1.38777878078144567553E-17       2**-56
 * MAXLOG =  8.8029691931113054295988E1       log(2**127)
 * MINLOG = -8.872283911167299960540E1        log(2**-128)
 * MAXNUM =  1.701411834604692317316873e38    2**127
 *
 * For IEEE arithmetic (IBMPC):
 * MACHEP =  1.11022302462515654042E-16       2**-53
 * MAXLOG =  7.09782712893383996843E2         log(2**1024)
 * MINLOG = -7.08396418532264106224E2         log(2**-1022)
 * MAXNUM =  1.7976931348623158E308           2**1024
 *
 * The global symbols for mathematical constants are
 * PI     =  3.14159265358979323846           pi
 * PIO2   =  1.57079632679489661923           pi/2
 * PIO4   =  7.85398163397448309616E-1        pi/4
 * SQRT2  =  1.41421356237309504880           sqrt(2)
 * SQRTH  =  7.07106781186547524401E-1        sqrt(2)/2
 * LOG2E  =  1.4426950408889634073599         1/log(2)
 * SQ2OPI =  7.9788456080286535587989E-1      sqrt( 2/pi )
 * LOGE2  =  6.93147180559945309417E-1        log(2)
 * LOGSQ2 =  3.46573590279972654709E-1        log(2)/2
 * THPIO4 =  2.35619449019234492885           3*pi/4
 * TWOOPI =  6.36619772367581343075535E-1     2/pi
 *
 * These lists are subject to change.
 */

/*							const.c */

/*
Cephes Math Library Release 2.0:  April, 1987
Copyright 1984, 1987 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"
#ifdef __GO32__
#undef IBMPC
#define UNK 1
#endif

#ifdef UNK
double MACHEP =  1.38777878078144567553E-17;   /* 2**-56 */
double MAXLOG =  8.8029691931113054295988E1;   /* log(2**127) */
double MINLOG = -8.872283911167299960540E1;    /* log(2**-128) */
double MAXNUM =  1.701411834604692317316873e38; /* 2**127 */
double CPI     =  3.14159265358979323846;       /* pi */
double PIO2   =  1.57079632679489661923;       /* pi/2 */
double PIO4   =  7.85398163397448309616E-1;    /* pi/4 */
double SQRT2  =  1.41421356237309504880;       /* sqrt(2) */
double SQRTH  =  7.07106781186547524401E-1;    /* sqrt(2)/2 */
double LOG2E  =  1.4426950408889634073599;     /* 1/log(2) */
double SQ2OPI =  7.9788456080286535587989E-1;  /* sqrt( 2/pi ) */
double LOGE2  =  6.93147180559945309417E-1;    /* log(2) */
double LOGSQ2 =  3.46573590279972654709E-1;    /* log(2)/2 */
double THPIO4 =  2.35619449019234492885;       /* 3*pi/4 */
double TWOOPI =  6.36619772367581343075535E-1; /* 2/pi */
/* almost 2^16384 */
long double MAXNUML = 1.189731495357231765021263853E4932L;
/* 2^-64 */
long double MACHEPL = 5.42101086242752217003726400434970855712890625E-20L;
/* log( MAXNUML ) */
long double MAXLOGL =  1.1356523406294143949492E4L;
/* log( underflow threshold = 2^(-16382) ) */
long double MINLOGL = -1.1355137111933024058873E4L;
long double LOGE2L  = 6.9314718055994530941723E-1L;
long double LOG2EL  = 1.4426950408889634073599E0L;
long double PIL     = 3.1415926535897932384626L;
long double PIO2L   = 1.5707963267948966192313L;
long double PIO4L   = 7.8539816339744830961566E-1L;
#endif

#ifdef IBMPC
			/* 2**-53 =  1.11022302462515654042E-16 */
unsigned short MACHEP[4] = {0x0000,0x0000,0x0000,0x3ca0};
			/* log(2**1024) =   7.09782712893383996843E2 */
unsigned short MAXLOG[4] = {0x39ef,0xfefa,0x2e42,0x4086};
			/* log(2**-1022) = - 7.08396418532264106224E2 */
unsigned short UNDLOG[4] = {0xbcd2,0xdd7a,0x232b,0xc086};
			/* log(2**-1074) = - -7.44440071921381262314E2 */
unsigned short MINLOG[4] = {0x71c3,0x446d,0x4385,0xc087};
			/* 2**1024*(1-MACHEP) =  1.7976931348623158E308 */
unsigned short MAXNUM[4] = {0xffffU,0xffffU,0xffffU,0x7fef};
unsigned short CPI[4]     = {0x2d18,0x5444,0x21fb,0x4009};
unsigned short PIO2[4]   = {0x2d18,0x5444,0x21fb,0x3ff9};
unsigned short PIO4[4]   = {0x2d18,0x5444,0x21fb,0x3fe9};
unsigned short SQRT2[4]  = {0x3bcd,0x667f,0xa09e,0x3ff6};
unsigned short SQRTH[4]  = {0x3bcd,0x667f,0xa09e,0x3fe6};
unsigned short LOG2E[4]  = {0x82fe,0x652b,0x1547,0x3ff7};
unsigned short SQ2OPI[4] = {0x3651,0x33d4,0x8845,0x3fe9};
unsigned short LOGE2[4]  = {0x39ef,0xfefa,0x2e42,0x3fe6};
unsigned short LOGSQ2[4] = {0x39ef,0xfefa,0x2e42,0x3fd6};
unsigned short THPIO4[4] = {0x21d2,0x7f33,0xd97c,0x4002};
unsigned short TWOOPI[4] = {0xc883,0x6dc9,0x5f30,0x3fe4};
unsigned short MAXNUML[] = {0xffffU,0xffffU,0xffffU,0xffffU,0x7ffe};
unsigned short MAXLOGL[] = {0x79ab,0xd1cf,0x17f7,0xb172,0x400c};
unsigned short MINLOGL[] = {0xeb2f,0x1210,0x8c67,0xb16c,0xc00c};
unsigned short MACHEPL[] = {0x0000,0x0000,0x0000,0x8000,0x3fbf};
unsigned short LOGE2L[]  = {0x79ac,0xd1cf,0x17f7,0xb172,0x3ffe};
unsigned short LOG2EL[]  = {0xf0bc,0x5c17,0x3b29,0xb8aa,0x3fff};
unsigned short PIL[]     = {0xc235,0x2168,0xdaa2,0xc90f,0x4000};
unsigned short PIO2L[]   = {0xc235,0x2168,0xdaa2,0xc90f,0x3fff};
unsigned short PIO4L[]   = {0xc235,0x2168,0xdaa2,0xc90f,0x3ffe};
#endif

#ifdef MIEEE
			/* 2**-53 =  1.11022302462515654042E-16 */
short MACHEP[4] = {
0x3ca0,0x0000,0x0000,0x0000
};
			/* log(2**1024) =   7.09782712893383996843E2 */
short MAXLOG[4] = {
0x4086,0x2e42,0xfefa,0x39ef
};
			/* log(2**-1022) = - 7.08396418532264106224E2 */
short MINLOG[4] = {
0xc086,0x232b,0xdd7a,0xbcd2
};
			/* 2**1024*(1-MACHEP) =  1.7976931348623158E308 */
short MAXNUM[4] = {
0x7fef,0xffff,0xffff,0xffff
};
short CPI[4]     = {
0x4009,0x21fb,0x5444,0x2d18
};
short PIO2[4]   = {
0x3ff9,0x21fb,0x5444,0x2d18
};
short PIO4[4]   = {
0x3fe9,0x21fb,0x5444,0x2d18
};
short SQRT2[4]  = {
0x3ff6,0xa09e,0x667f,0x3bcd
};
short SQRTH[4]  = {
0x3fe6,0xa09e,0x667f,0x3bcd
};
short LOG2E[4]  = {
0x3ff7,0x1547,0x652b,0x82fe
};
short SQ2OPI[4] = {
0x3fe9,0x8845,0x33d4,0x3651
};
short LOGE2[4]  = {
0x3fe6,0x2e42,0xfefa,0x39ef
};
short LOGSQ2[4] = {
0x3fd6,0x2e42,0xfefa,0x39ef
};
short THPIO4[4] = {
0x4002,0xd97c,0x7f33,0x21d2
};
short TWOOPI[4] = {
0x3fe4,0x5f30,0x6dc9,0xc883
};
long MAXNUML[] = {0x7ffe0000,0xffffffff,0xffffffff};
long MAXLOGL[] = {0x400c0000,0xb17217f7,0xd1cf79ab};
long MINLOGL[] = {0xc00c0000,0xb16c8c67,0x1210eb2f};
long MACHEPL[] = {0x3fbf0000,0x00000000,0x00000000};
long LOGE2L[]  = {0x3ffe0000,0xb17217f7,0xd1cf79ac};
long LOG2EL[]  = {0x3fff0000,0xb8aa3b29,0x5c17f0bc};
long PIL[]     = {0x40000000,0xc90fdaa2,0x2168c235};
long PIO2L[]   = {0x3fff0000,0xc90fdaa2,0x2168c235};
long PIO4L[]   = {0x3ffe0000,0xc90fdaa2,0x2168c235};

#endif

#ifdef DEC
			/* 2**-56 =  1.38777878078144567553E-17 */
short MACHEP[4] = {0022200,0000000,0000000,0000000};
			/* log 2**127 = 88.029691931113054295988 */
short MAXLOG[4] = {041660,007463,0143742,025733,};
			/* log 2**-128 = -88.72283911167299960540 */
short MINLOG[4] = {0141661,071027,0173721,0147572,};
			/* 2**127 = 1.701411834604692317316873e38 */
short MAXNUM[4] = {077777,0177777,0177777,0177777,};
short CPI[4]     = {040511,007732,0121041,064302,};
short PIO2[4]   = {040311,007732,0121041,064302,};
short PIO4[4]   = {040111,007732,0121041,064302,};
short SQRT2[4]  = {040265,002363,031771,0157145,};
short SQRTH[4]  = {040065,002363,031771,0157144,};
short LOG2E[4]  = {040270,0125073,024534,013761,};
short SQ2OPI[4] = {040114,041051,0117241,0131204,};
short LOGE2[4]  = {040061,071027,0173721,0147572,};
short LOGSQ2[4] = {037661,071027,0173721,0147572,};
short THPIO4[4] = {040426,0145743,0174631,007222,};
short TWOOPI[4] = {040042,0174603,067116,042025,};
#endif
/*
extern short MACHEP[];
extern short MAXLOG[];
extern short UNDLOG[];
extern short MINLOG[];
extern short MAXNUM[];
extern short CPI[];
extern short PIO2[];
extern short PIO4[];
extern short SQRT2[];
extern short SQRTH[];
extern short LOG2E[];
extern short SQ2OPI[];
extern short LOGE2[];
extern short LOGSQ2[];
extern short THPIO4[];
extern short TWOOPI[];
*/
#if 0
/* Pacify compiler: -Wunused-variable */
static char x[] =
"Cephes Math Library Copyright 1987 by Stephen L. Moshier";
#endif
