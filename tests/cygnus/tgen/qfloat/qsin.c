
/*							qsin.c	*/
/* circular sine check routine */

#include "qhead.h"
extern QELT qone[], qpi[];

int qsin( x, y )
QELT *x, *y;
{
QELT a[NQ], b[NQ], z[NQ], xx[NQ];
int sign;
long mod;

if( x[0] != 0 )
	sign = -1;
else
	sign = 1;
qmov( x, xx );
xx[0] = 0;
/* range reduction to [0, pi/2]	*/
qmov( qpi, z );
z[1] -= 1;
qdiv( z, xx, a );
qfloor( a, xx );
/* mod = dmod - 8 * (dmod/8) */
if( xx[1] >= 3)
	{
	xx[1] -= 3;
	qfloor( xx, b );
	b[1] += 3;
	xx[1] += 3;
	}
else
	qclear(b);

qsub( b, xx, b );
qifrac( b, &mod, b );

qsub( xx, a, b );
qmul( b, z, xx );

mod &= 3;
if( mod > 1 )
	sign = -sign;
if( mod & 1 )
	qsub( xx, z, xx );	/* xx = 1 - xx */

qmul( xx, xx, z );
qneg( z );

qmov( qone, a );
qmov( qone, b );
qmov( qone, y );

/* power series */
do
	{
	qadd( qone, a, a );	/* a += 1	*/
	qdiv( a, b, b );	/* b /= a	*/
	qadd( qone, a, a );	/* a += 1	*/
	qdiv( a, b, b );	/* b /= a	*/
	qmul( z, b, b );	/* b *= z	*/
	qadd( b, y, y );	/* y += b	*/
	}
while( (int)(y[1] - b[1]) < NBITS );

qmul( xx, y, y );
if( sign < 0 )
	y[0] = -1;
return 0;
}



int qsinmx3( x, y )
QELT *x, *y;
{
QELT z[NQ], b[NQ], n[NQ];

qmul( x, x, z );
qneg( z );

qmov( qone, n );
qclear( y );

/* compute the cube term x^3/3! */
qmov( x, b );
qadd( qone, n, n );
qdiv( n, b, b );
qadd( qone, n, n );
qdiv( n, b, b );
qmul( z, b, b );

/* power series */
do
	{
	qadd( qone, n, n );	/* n += 1	*/
	qdiv( n, b, b );	/* b /= n	*/
	qadd( qone, n, n );	/* n += 1	*/
	qdiv( n, b, b );	/* b /= n	*/
	qmul( z, b, b );	/* b *= z	*/
	qadd( b, y, y );	/* y += b	*/
	}
while( (int)(y[1] - b[1]) < NBITS );
return 0;
}
