/*							qtanh.c		*/
/* hyperbolic tangent check routine */
/* this subroutine is used by the exponential function routine */

/* loop count adjusted for convergence to 9 word mantissa if x <= 2 */

#include "qhead.h"

extern QELT qone[], qtwo[];

int qtanh( x, y )
QELT *x, *y;
{
QELT e[NQ], r[NQ], j[NQ], xx[NQ], m2[NQ];
int i, n;
long lj;

qmov( x, r );
r[0] = 0;
if( qcmp(r, qone) >= 0 )
	{
/* tanh(x) = (exp(x) - exp(-x)) / (exp(x) + exp(-x))
 * Note qexp() calls qtanh, but with an argument less than (1 + log 2)/2.
 */
	qexp( r, e );
	qdiv( e, qone, r );
	qsub( r, e, xx );
	qadd( r, e, j );
	qdiv( j, xx, y );
	goto done;
	}

qmov( qtwo, m2 );
qneg( m2 );

n = NBITS/9 + 1; /*10;*/
lj = 2 * n + 1;
ltoq( &lj, j );

qmov( j, e );
qmul( x, x, xx );

/* continued fraction */

/*for( i=0; i<15; i++)*/


for( i=0; i<n; i++)
	{
	qdiv( e, xx, r );
	qadd( m2, j, j );
	qadd( r, j, e );
	}

qdiv( e, x, y );

done:
if( x[0] != 0 )
	y[0] = -1;
return 0;
}
