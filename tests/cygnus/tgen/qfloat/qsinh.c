/*							qsinh.c		*/
/* hyperbolic sine check routine  */
#include "qhead.h"
extern QELT qone[];

int qsinh( x, y )
QELT *x, *y;
{
QELT xx[NQ], qn[NQ], qf[NQ], qz[NQ];

if( x[1] < 2 )
	{
	qclear( y );
	return 0;
	}
if( x[1] < (QELT) (EXPONE - 1) )
	{
	qmul( x, x, xx );
	qmov( qone, qz );
	qmov( qone, qf );
	qmov( qone, qn );
	do
		{
		qadd( qone, qn, qn );
		qdiv( qn, qf, qf );
		qadd( qone, qn, qn );
		qdiv( qn, qf, qf );
		qmul( xx, qf, qf );
		qadd( qz, qf, qz );
		}
	while( (int)(qz[1] - qf[1]) < NBITS );
	qmul( x, qz, y );
	return 0;
	}

qexp( x, qz );
qdiv( qz, qone, y );
qsub( y, qz, y );
*(y+1) -= 1;
return 0;
}
