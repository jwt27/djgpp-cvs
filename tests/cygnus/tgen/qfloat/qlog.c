/*						qlog.c	*/
/* natural logarithm */

#include "qhead.h"

extern QELT qone[], qtwo[], qlog2[], qsqrt2[];

/* C1 + C2 = ln 2 */
#if WORDSIZE == 16
static QELT C1[NQ] = {0,EXPONE-1,0,0xb172, 0x17f7};
#if NQ > 12
static QELT C2[NQ] = {
0x0000,EXPONE-33,0x0000,0xd1cf,0x79ab,0xc9e3,0xb398,0x03f2,
0xf6af,0x40f3,0x4326,0x7298,0xb62d,0x8a0d,0x175b,0x8baa,
0xfa2b,0xe7b8,0x7620,0x6deb,0xac98,0x5595,0x52fb,0x4afa};
#else /* not NQ > 12 */
static QELT C2[NQ] = {
0x0000,EXPONE-33,0x0000,0xd1cf,0x79ab,0xc9e3,0xb398,0x03f2,
0xf6af,0x40f3,0x4326,0x7299};
#endif /* not NQ > 12 */
#else /* WORDSIZE 32 */
static QELT C1[NQ] = {0,EXPONE-1,0,0xb17217f7};
static QELT C2[NQ] = {
0x0000,EXPONE-33,0x0000,0xd1cf79ab,0xc9e3b398,0x03f2f6af,0x40f34326,0x7298b62d,
0x8a0d175b,0x8baafa2b,0xe7b87620,0x6debac98,0x559552fb,0x4afa1b11};
#endif /* WORDSIZE 32 */

int qlog( x, y )
QELT *x, *y;
{
QELT xx[NQ], z[NQ], a[NQ], b[NQ], t[NQ], qj[NQ];
long ex;

if( x[0] != 0 )
	{
	qclear(y);
	mtherr( "qlog", DOMAIN );
	return 0;
	}
if( x[1] == 0 )
	{
	qinfin( y );
	y[0] = -1;
	mtherr( "qlog", SING );
	return 0;
	}
/* range reduction: log x = log( 2**ex * m ) = ex * log2 + log m */
qmov(x, xx );
ex = *(xx+1);
if( ex == EXPONE )
	{ /* log 1 = 0 */
	if( qcmp(x, qone) == 0 )
		{
		qclear(y);
		return 0;
		}
	}
ex -= (EXPONE-1);
xx[1] = (EXPONE-1);
/* Adjust range to 1/sqrt(2), sqrt(2) */
qsqrt2[1] -= 1;
if( qcmp( xx, qsqrt2 ) < 0 )
	{
	ex -= 1;
	xx[1] += 1;
	}
qsqrt2[1] += 1;

qadd( qone, xx, b );
qsub( qone, xx, a );
if( a[1] == 0 )
	{
	qclear(y);
	goto bdone;
	}
qdiv( b, a, y );	/* store (x-1)/(x+1) in y */

qmul( y, y, z );

qmov( qone, a );
qmov( qone, b );
qmov( qone, qj );
do
	{
	qadd( qtwo, qj, qj );	/* 2 * i + 1		*/
	qmul( z, a, a );
	qdiv( qj, a, t );
	qadd( t, b, b );
	}
while( ((int) b[1] - (int) t[1]) < NBITS );


qmul( b, y, y );
y[1] += 1;

bdone:

/* now add log of 2**ex */
if( ex != 0 )
	{
	ltoq( &ex, b );
	qmul( C2, b, t );
	qadd( t, y, y );
	qmul( C1, b, t );
	qadd( t, y, y );
	}
return 0;
}
